
[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-preferences-window.ui")]
public sealed class Bedit.PreferencesWindow : Gtk.Window {
//    private GLib.Settings settings = new GLib.Settings("com.bwhmather.Bedit2");

    public PreferencesWindow(Gtk.Application application) {
        Object(application: application);
    }
}
