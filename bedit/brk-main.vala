namespace Brk {

private bool initialized = false;
private GLib.HashTable<Gdk.Display, Brk.StyleManager> display_style_managers;

private void
register_display(Gdk.Display display) {
    return_if_fail(!display_style_managers.contains(display));
    
    var style_manager = new Brk.StyleManager(display);
    display_style_managers.insert(display, style_manager);
    display.closed.connect((d, is_error) => { unregister_display(d); });
}

private void
unregister_display(Gdk.Display display) {
    return_if_fail(display_style_managers.contains(display));
    display_style_managers.remove(display);
}

public void
init() {
    if (initialized) {
        return;
    }

    Gtk.init();

    Intl.bind_textdomain_codeset(Config.GETTEXT_PACKAGE, "UTF-8");
    Intl.bindtextdomain(Config.GETTEXT_PACKAGE, Config.LOCALEDIR);

    display_style_managers = new GLib.HashTable<Gdk.Display, Brk.StyleManager>(
        GLib.direct_hash, GLib.direct_equal
    );

    var display_manager = Gdk.DisplayManager.get();
    var displays = display_manager.list_displays();
    for (unowned var l = displays; l != null; l = l.next) {
        Brk.register_display(l.data);
    }

    display_manager.display_opened.connect((m, d) => { Brk.register_display(d); });

    initialized = true;
}

public bool
is_initialized() {
    return initialized;
}

}

internal sealed class Brk.StyleManager : GLib.Object {
    public Gdk.Display display { get; construct; }

    public StyleManager(Gdk.Display display) {
        Object(display: display);
    }
}
