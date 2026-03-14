# OpenURI Portal

The OpenURI portal allows applications to open URIs (e.g. `https://`,
`file://`, or `mailto:`).

## Description

-   To open a URI, an application should
    -   connect to the socket at `$XDG_RUNTIME_DIR/xi.portal.OpenURI`
    -   send the URI
    -   read the response
    -   close the connection
-   On success, the response is an empty string.
-   On error, the response is a human-readable error message.
-   `file://` URIs must be interpreted in the mount namespace of the calling
    process

Example usage:

```sh
$ printf "https://example.com" | nc -U "$XDG_RUNTIME_DIR/xi.portal.OpenURI"`
```

## Open Questions

-   How to pass [activation tokens](https://wayland.app/protocols/xdg-activation-v1)?
-   It is not clear how support for `file://` URIs should be implemented. The
    namespace is available at `/proc/{pid}/root/`, but that will go away when
    the calling process exits.

## Compatibility Considerations

This portal and the corresponding XDG Desktop Portals can coexist without
issue. However, it is not easily possible to implement one on top of the other
because of the very different handling of `file://` URIs.

## Security Considerations

-   Applications might cause Denial of Service by excessive use. The portal
    service may implement rate limiting.
-   Applications might exfiltrate data by passing parameters to an
    attacker-controlled domain. This is especially relevant if simpler means of
    data exfiltration (i.e. network access) is blocked. In those cases, access
    to this portal should be blocked as well.
-   Applications might try to open malicious URIs. The portal should refuse to
    open potentially dangerous URIs (e.g. executable files).
-   In the case of `file://` URIs, the portal must not leak information (e.g.
    PID, host path) in the error message.

## Related Links

-   [org.freedesktop.portal.OpenURI](https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.OpenURI.html)
-   [org.freedesktop.portal.Email](https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.Email.html)
-   [RFC 6068 - The 'mailto' URI Scheme](https://www.rfc-editor.org/rfc/rfc6068)
