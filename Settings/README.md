# Settings Portal

This interface provides access to settings. It describes both a general purpose
mechanism for making settings available to applications as well as defining a
small number of standardized host settings required for toolkits.

## General Purpose Mechanism

-   Settings are stored in files inside
    [`$XDG_CONFIG_HOME`](https://specifications.freedesktop.org/basedir/latest/).
-   Sandboxes can give read-write, read-only, or no access to a given settings
    file.
-   Sandboxes may also present different settings than what is used on the host
    system.
-   Applications can watch settings using `inotify(7)`

## Standardized Settings

-   `$XDG_CONFIG_HOME/org.freedesktop.appearance/color-scheme` (`''`, `'dark'`,
    `'light'`)
-   `$XDG_CONFIG_HOME/org.freedesktop.appearance/accent-color` (`''`,
    `#rrggbb`)
-   `$XDG_CONFIG_HOME/org.freedesktop.appearance/contrast` (`''`, `'high'`)
-   `$XDG_CONFIG_HOME/org.freedesktop.appearance/reduced-motion` (`''`,
    `'reduced'`)

## Open Questions

-   Should `inotify` be used to watch directories or files? That makes a
    difference when files are not only modified, but also deleted, created, or
    moved.
-   Should there be one setting per file (`/proc/`-style) or multiple settings
    in a single file?
-   Should sandboxes be allowed to bind individual files? (can mess with
    inotify)

## Compatibility Considerations

Legacy applications may either use `org.freedesktop.portal.Settings` or
`gsettings`.

It should be easily possible to implement a service that watches the relevant
settings and syncs them to gsettings. This service could also implement the
relevant xdg portal backend or rely on `xdg-desktop-portal-gtk`.

## Security Considerations

-   The available set of settings should be limited, well-structured, and
    clearly labelled so that users understand which settings they should expose
    to sandboxed applications.
-   Sandboxes should usually expose settings read-only.
-   Confidential data (e.g. passwords) should not be stored in settings.
-   Settings that belong to different security domains should be stored in
    different files.

## Related Links

-   [org.freedesktop.portal.Settings](https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.Settings.html)
