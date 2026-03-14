# Secrets Portal

The Secrets portal allows to store passwords and other secrets.

## Description

-   A key is stored in `$XDG_DATA_HOME/xi.portal.Secret/key` on the host,
    encrypted with a password.
-   Secrets are stored in `$XDG_DATA_HOME/xi.portal.Secret/secrets` in the
    application's mount namespace, encrypted with the key.
-   Sandboxes may mount a different directory to
    `$XDG_DATA_HOME/xi.portal.Secret` to isolate an application's secrets. That
    should be a subdirectory of `$XDG_DATA_HOME/xi.portal.Secret` on the host.
-   To access keys, applications talk to `$XDG_RUNTIME_DIR/xi.portal.Secret`.

## Open Questions

-   Define a protocol, possibly based on [Himitsu](https://git.sr.ht/~sircmpwn/himitsu/tree/master/item/docs/himitsu-ipc.5.scd)

## Compatibility Considerations

This approach is not compatible with Flatkap, because its use of xdg-dbus-proxy
obscures the calling application's mount namespace. The portal should
therefore implement `org.freedesktop.Secret` and
`org.freedesktop.portal.Secret` in addition to the interfaces described in this
document.

## Security Considerations

-   Implementations of this portal should be transparent about their exact
    threat model so users can make informed decisions about if and how they
    want to use it.
-   Care must be taken to avoid race conditions when accessing the
    application's mount namespace.

## Related Links

-   [org.freedesktop.Secret](https://specifications.freedesktop.org/secret-service/)
-   [org.freedesktop.portal.Secret](https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.Secret.html)
-   [The plan to support per-app passwords on the linux desktop](https://blog.ce9e.org/posts/2024-07-27-password-plan/)
-   [Himitsu](https://git.sr.ht/~sircmpwn/himitsu/)
-   [xi-keyring](https://github.com/xi/xi-keyring)
