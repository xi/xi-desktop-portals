# StatusIndicator Portal

This interface allows applications to indicate their status and provide quick
access to common actions.

## Description

-   Applications can create a status indicator by connecting to
    `$XDG_RUNTIME_DIR/xi.portal.StatusIndicator`.
-   Status indicators may contain a list of actions. These action may be
    displayed as a menu. When an action is triggered, it is send back to the
    application.
-   When the connection is closed, the status indicator is removed.


## Open Questions

-   Define the exact protocol.

## Compatibility Considerations

While XDG Desktop Portals only provides notifications, but no status
indicators, this project takes the opposite approach (see
[notification-hub](https://github.com/xi/notification-hub) for motivation).
However, the two concepts are mostly compatible: Notifications can be displayed
as persistent status indicators, and status messages can be displayed as
transient notifications.

## Security Considerations

-   Applications may try to cause Denial of Service by opening many simultanious
    connections.
-   Actions are never executed by the host, so there is no risk of privilege
    escalation.

## Related Links

-   [org.freedesktop.StatusNotifierItem](https://www.freedesktop.org/wiki/Specifications/StatusNotifierItem/)
-   [org.freedesktop.Notifications](https://specifications.freedesktop.org/notification/latest/)
-   [org.freedesktop.portal.Notification](https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.Notification.html)
-   [notification-hub](https://github.com/xi/notification-hub)
