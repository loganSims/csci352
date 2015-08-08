#include <gtk/gtk.h>
#include "access.h"

static void display (GtkWidget* hpaned);

static void add_info_dirstore(GtkListStore *dirstore, char *path);
static void build_dirview(GtkWidget *dirview);

void dir_item_selected (GtkWidget *selection, gpointer data);

enum {
  NAME_COLUMN = 0,
  SIZE_COLUMN,
  DATE_COLUMN,
  PERM_COLUMN,
  ICON_COLUMN,
  N_COLUMNS
};

const gchar* colnames[] = { "Name", "Size", "Date", "Permissions" };

int main (int argc, char *argv[]){

  GtkWidget *hpaned, *vpaned, *dirview;

  gtk_init (&argc, &argv);

  //filler
  GtkWidget *button1 = gtk_button_new_with_label ("sys");
  GtkWidget *button2 = gtk_button_new_with_label ("hex");
  //end filler

  // setup dirstore
  GtkListStore *dirstore = gtk_list_store_new(N_COLUMNS,
                                              G_TYPE_STRING, /*File name   */
                                              G_TYPE_STRING, /*File size   */
                                              G_TYPE_STRING, /*Mod Date    */
                                              G_TYPE_STRING, /*Access Perm */
                                              GDK_TYPE_PIXBUF); /*Icon        */

  add_info_dirstore(dirstore, "/");

  // setup dirview
  dirview = gtk_tree_view_new ();
  build_dirview(dirview);

  //connect dirview and dirstore
  gtk_tree_view_set_model (GTK_TREE_VIEW (dirview), 
                           GTK_TREE_MODEL (dirstore));



  //setup panes & pack them
  hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  vpaned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
  gtk_widget_set_size_request(hpaned, 200, 500);
  gtk_widget_set_size_request(vpaned, 500, 500);

  gtk_paned_pack1 (GTK_PANED (hpaned), button1, TRUE, FALSE);
  gtk_paned_pack2 (GTK_PANED (hpaned), vpaned, FALSE, FALSE);

  // create a scrolled window for dirview
  GtkWidget* scroller = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroller),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);

  gtk_container_add (GTK_CONTAINER (scroller), dirview);

  // pack vpaned
  gtk_paned_pack1 (GTK_PANED (vpaned), scroller, TRUE, FALSE);
  gtk_paned_pack2 (GTK_PANED (vpaned), button2, TRUE, FALSE);

  display(hpaned);

  gtk_main ();
  return 0;
}

static void add_info_dirstore(GtkListStore *dirstore, char *path){

  GtkTreeIter iter;
  DIR *dp;
  struct stat sb;
  struct dirent *entry;
  struct tm time;
  char entrypath[256];
  char modtime[9];
  char permission[10];
  char size[15];
  GError *error = NULL;
  GdkPixbuf* dir_icon = gdk_pixbuf_new_from_file("dir_icon.png", &error);
  GdkPixbuf* file_icon = gdk_pixbuf_new_from_file("file_icon.png", &error);
  GdkPixbuf* exe_icon = gdk_pixbuf_new_from_file("icon2.png", &error);
  GdkPixbuf* c_icon = gdk_pixbuf_new_from_file("icon2.png", &error);
  GdkPixbuf* unknown_icon = gdk_pixbuf_new_from_file("icon2.png", &error);
  GdkPixbuf* icon;

  dp = opendir(path);

  while((entry = readdir(dp))){

    // Get path of entry for stat
    snprintf(entrypath, 256, "%s%s", path, entry->d_name);
    stat(entrypath, &sb); 

    // Format time
    localtime_r(&(sb.st_mtime), &time);
    strftime(modtime, sizeof(modtime), "%D", &time);
    
    // assemble permission string
    makePerm(&sb, permission);

    // format size
    snprintf(size, 15, "%.1f %s", 
        ((sb.st_size < 1000) ? sb.st_size : (sb.st_size/1000.0)),
        ((sb.st_size < 1000) ? "B" : "KiB") );
    gtk_list_store_append (dirstore, &iter);

    // setup icon  TODO c file, exe file, ...
    switch (sb.st_mode & S_IFMT) {
      case S_IFDIR: icon = dir_icon; break; //directory
      case S_IFREG: icon = file_icon; break; //reg file
      default:  icon = unknown_icon; break; //unknown
    }

    gtk_list_store_set (dirstore, &iter,
                        NAME_COLUMN, entry->d_name,
                        SIZE_COLUMN, size,
                        DATE_COLUMN, modtime,
                        PERM_COLUMN, permission, 
                        ICON_COLUMN, icon, -1);

  }

}

static void build_dirview(GtkWidget *dirview){

  int i;
  int j = 0;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  for(i = NAME_COLUMN; i <= PERM_COLUMN; i++){

    column = gtk_tree_view_column_new();

    if (j == NAME_COLUMN){
      renderer = gtk_cell_renderer_pixbuf_new();
      gtk_tree_view_column_pack_start(column, renderer, FALSE);
      gtk_tree_view_column_set_attributes(column, renderer, 
                                          "pixbuf", ICON_COLUMN, NULL);
    }

    renderer = gtk_cell_renderer_text_new ();

    if (j == SIZE_COLUMN){
       g_object_set (renderer, "xalign", 1.0, NULL);
    }

    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column, renderer, 
                                         "text", i, NULL); 

    gtk_tree_view_column_set_title(column, colnames[j]);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_min_width (column, 60);
    gtk_tree_view_column_set_max_width (column, 200);

    if (j != PERM_COLUMN){
      gtk_tree_view_column_set_sort_indicator(column, TRUE);           
      gtk_tree_view_column_set_sort_column_id(column, j);
    }

    gtk_tree_view_append_column (GTK_TREE_VIEW (dirview), column);
    j++;
  }

  // add selection handling
  GtkTreeSelection *selection;
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(dirview));
    
  // connect the selection callback function
  g_signal_connect (G_OBJECT(selection), "changed", 
                    G_CALLBACK(dir_item_selected), NULL);

}


// Start hex viewer
void dir_item_selected (GtkWidget *selection, gpointer data) {
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected (GTK_TREE_SELECTION(selection), 
        &model, &iter)) {
        
        gchar *name;
        gtk_tree_model_get (model, &iter, NAME_COLUMN, &name, -1);
        g_message("selected %s\n", name);
    }
}


void display (GtkWidget *hpaned) {
    
  // create the window
  GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "File Explorer");
  gtk_container_set_border_width (GTK_CONTAINER (window), 10);
  gtk_widget_set_size_request (window, 700, 500);
  g_signal_connect (window, "delete_event", gtk_main_quit, NULL);
    
  // pack the containers
  gtk_container_add (GTK_CONTAINER (window), hpaned);
  gtk_widget_show_all (window);
}


