use anyhow::{anyhow, Context, Result};
use gio::prelude::FileExt;
use libc::pid_t;
use nix::fcntl::{open, openat2, OFlag, OpenHow, ResolveFlag};
use nix::poll::{poll, PollFd, PollFlags, PollTimeout};
use nix::sys::socket::{getsockopt, sockopt};
use nix::sys::stat::{fstat, FileStat, Mode};
use std::io::{Read, Write};
use std::os::fd::{AsFd, FromRawFd, OwnedFd};
use std::os::unix::io::RawFd;
use std::os::unix::net::UnixStream;
use std::path::{Path, PathBuf};

const SOCK_FD: RawFd = 3;

fn check_running(pidfd: &OwnedFd) -> Result<()> {
    let mut pollfds = [PollFd::new(pidfd.as_fd(), PollFlags::POLLIN)];
    let nready = poll(&mut pollfds, PollTimeout::ZERO)?;
    if nready != 0 {
        return Err(anyhow!(
            "The client's mount namespace is no longer available"
        ));
    }
    Ok(())
}

fn stat_at_root(path: &Path, root: &Path) -> Result<FileStat> {
    let root_fd = open(root, OFlag::O_RDONLY | OFlag::O_DIRECTORY, Mode::empty())?;

    let how = OpenHow::new()
        .flags(OFlag::O_RDONLY)
        .resolve(ResolveFlag::RESOLVE_IN_ROOT);
    let fd = openat2(root_fd, path, how)?;

    let st = fstat(fd)?;
    Ok(st)
}

fn same_on_host(path: &Path, pid: pid_t, pidfd: OwnedFd) -> Result<()> {
    let mut sender_root = PathBuf::from("/proc");
    sender_root.push(pid.to_string());
    sender_root.push("root");

    let st_sender = stat_at_root(path, sender_root.as_path())?;
    check_running(&pidfd)?;

    if let Ok(st_host) = stat_at_root(path, Path::new("/")) {
        if st_sender.st_dev == st_host.st_dev && st_sender.st_ino == st_host.st_ino {
            return Ok(());
        }
    }
    Err(anyhow!("Not the same file on the host"))
}

fn read_input(stream: &mut UnixStream) -> Result<String> {
    let mut buf = vec![];
    loop {
        let mut tmp = [0u8; 1024];
        let n = stream.read(&mut tmp)?;
        buf.append(&mut tmp[..n].to_vec());
        if n < tmp.len() {
            break;
        }
    }
    if buf.pop().context("Invalid input: empty")? != b'\n' {
        return Err(anyhow!("Invalid input: newline"));
    }
    let input = String::from_utf8(buf)?;
    Ok(input)
}

fn read_and_open(stream: &mut UnixStream) -> Result<()> {
    stream.set_nonblocking(true)?;
    let mut uri = read_input(stream)?;

    if uri.starts_with('/') {
        uri = gio::File::for_path(uri).uri().to_string();
    }

    if uri.starts_with("file://") {
        let pid = getsockopt(&stream, sockopt::PeerCredentials)?.pid();
        let pidfd = getsockopt(&stream, sockopt::PeerPidfd)?;
        let path = gio::File::for_uri(&uri).path().unwrap();
        same_on_host(&path, pid, pidfd)?;
    }

    gio::AppInfo::launch_default_for_uri(&uri, gio::AppLaunchContext::NONE)?;

    Ok(())
}

fn main() {
    let mut stream = unsafe { UnixStream::from_raw_fd(SOCK_FD) };

    if let Err(e) = read_and_open(&mut stream) {
        stream.write_all(format!("{}\n", e).as_bytes()).unwrap();
    }
}
