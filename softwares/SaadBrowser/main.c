#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

static const char *candidates[] = {
  "file:///opt/Saad-Browser/resources/index.html",
  "file://./renderer/index.html",
  "file://./index.html"
};

int main(int argc, char *argv[]) {
  gtk_init (&argc, &argv);



  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Saad Browser");
  gtk_window_set_default_size(GTK_WINDOW(window), 900, 650);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);



  WebKitWebView *view = WEBKIT_WEB_VIEW(webkit_web_view_new());
  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(view));

  for (guint i = 0; i < G_N_ELEMENTS(candidates); i++) {
    const char *uri = candidates[i];
    const char *path = uri + 7; // skip "file://"
    if (g_file_test(path, G_FILE_TEST_EXISTS)) {
      webview(view, uri);
      break;
    }
  }




  gtk_widget_show_all(window);
  gtk_main();



  return 0;



}
