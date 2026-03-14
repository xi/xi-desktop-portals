# Desktop Portals

Simpler specifications for Linux desktop APIs.

[XDG Desktop Portals](https://flatpak.github.io/xdg-desktop-portal/) do a good
job at standardizing Linux desktop APIs. However, they are not perfect either.
So this is a space to explore ideas beyond XDG Desktop Portals.

## Goals

### Security Boundaries as Files Access

Mount namespaces are great. The question "does this application have permission
to use this API" should therefore usually be expressed as "does this process
have access to this file?".

### Meaningful User Interaction

Access to privileged interfaces should require a user interaction.
To avoid [warning fatigue](https://en.wikipedia.org/wiki/Alarm_fatigue), these
interactions should not just be confirmation dialogs, but offer [meaningful
interaction](https://youtu.be/bIzJyp8sb70?si=rt0cVjmeg5A2W7MF&t=934). They
should not just ask *if* an action may happen, but *how* it should happen.

Actions that are *triggered* by a user interaction (e.g. taking a screenshot)
are out of scope for this project because they should be provided by
compositors or by [privileged wayland
clients](https://blog.ce9e.org/posts/2025-10-03-wayland-security/). The
relevant interfaces also need to be standardized, but that is a separate topic.

### Compatibility and Security Considerations

Every interface should have clear documentation on compatibility with existing
systems (including XDG Desktop Portals) and its threat model.

### Avoid Dependencies

While the XDG Desktop Portal developers do a good job of providing a [uniform
platform API](https://www.youtube.com/watch?v=CHNJFkzIsxA), it is often clear
to see that those are the same people who also work on Flatpak and Gnome.
Notably, [only a small number of sandboxing
mechanisms](https://github.com/flatpak/xdg-desktop-portal/issues/1382) are
supported.

This project, on the other hand, should be truly independent. Dependency on
other parts of the systems should be kept to a minimum. Where not avoidable, it
should be clearly documented.

### Avoid D-Bus

I believe D-Bus is a major factor why many projects beyond the big desktop
environments do not adopt portals. It is complex and verbose and generally no
fun to work with, while providing little benefits.

The main issue with D-Bus is that it breaks the
security-boundaries-as-file-access rule: All interfaces are accesible on a
single socket. XDG Desktop Portals need to rely on hacks like
[xdg-dbus-proxy](https://github.com/flatpak/xdg-dbus-proxy) to work around
that.

### Avoid Authentication

Many XDG Desktop Portals require that they have a reliable way to get an app ID
for the calling process. They then use this ID e.g. to check permissions or to
show a corresponding app icon.

This approach has a couple of issues. For one, there is no standard for
providing those app IDs, so there is special code for each supported sandboxing
mechanism. What is worse, each process can only have a single app ID. For
nested sandboxes, only the outer-most app ID is used. That means that a
sandboxes application can not launch children that are even more restricted.

This project, on the other hand, tries to see how far we can get by only using
security-boundaries-as-file-access.
