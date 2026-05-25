use anyhow::{Context, Result};
use gio;
use gio::prelude::{FileEnumeratorExt, FileExt, FileMonitorExt, SettingsExt};
use glib;
use std::fs;
use std::path::PathBuf;

fn on_change(
    settings: &gio::Settings,
    settings_a11y: &gio::Settings,
    file: &gio::File,
) -> Result<()> {
    let path = file.path().context("file has no local path")?;
    let key = path.file_name().context("file has no name")?;

    let contents = fs::read_to_string(&path)?;
    let value = contents.trim();

    match key.to_str() {
        Some("color-scheme") => {
            let val = match value {
                "dark" => "prefer-dark",
                "light" => "prefer-light",
                _ => "default",
            };
            settings.set_string("color-scheme", val)?;
        }
        Some("accent-color") => {
            // TODO: translate color to color name
            settings.set_string("accent-color", value)?;
        }
        Some("reduced-motion") => {
            let val = match value {
                "reduced" => "reduced",
                _ => "no-preference",
            };
            settings_a11y.set_string("reduced-motion", val)?;
        }
        Some("contrast") => {
            let high = value == "high";
            settings_a11y.set_boolean("high-contrast", high)?;
        }
        _ => {}
    }
    return Ok(());
}

fn scan_initial_files(
    settings: &gio::Settings,
    settings_a11y: &gio::Settings,
    dir: &gio::File,
) -> Result<()> {
    let enumerator = dir.enumerate_children(
        "standard::name",
        gio::FileQueryInfoFlags::NONE,
        gio::Cancellable::NONE,
    )?;

    for entry in enumerator.clone() {
        if let Ok(info) = entry {
            let child = enumerator.child(&info);
            let _ = on_change(settings, settings_a11y, &child);
        }
    }

    return Ok(());
}

fn main() -> Result<()> {
    let settings = gio::Settings::new("org.gnome.desktop.interface");
    let settings_a11y = gio::Settings::new("org.gnome.desktop.a11y.interface");

    let root: PathBuf = {
        let cfg = glib::user_config_dir();
        cfg.join("org.freedesktop.appearance")
    };
    let dir = gio::File::for_path(&root);

    let monitor = dir.monitor_directory(gio::FileMonitorFlags::NONE, gio::Cancellable::NONE)?;

    let s = settings.clone();
    let sa = settings_a11y.clone();
    monitor.connect_changed(move |_monitor, file, _other, event_type| {
        if event_type == gio::FileMonitorEvent::ChangesDoneHint {
            let _ = on_change(&s, &sa, &file);
        }
    });

    scan_initial_files(&settings, &settings_a11y, &dir)?;

    glib::MainLoop::new(None, false).run();

    return Ok(());
}
