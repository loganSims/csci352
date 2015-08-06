#include <gtk/gtk.h>
#include "access.h"

static void destroy (GtkWidget*, gpointer);
static void setupWindow(GtkWidget**);

static void addDirInfoToStore(GtkListStore **dirstore, GtkWidget **dirview);
static void renderDirview(GtkWidget **dirview);

enum {
  NAME_COLUMN,
  SIZE_COLUMN,
  DATE_COLUMN,
  PERM_COLUMN,
  COLOR_COLUMN,
  N_COLUMNS
};

#define N_COLUMNS 4

const gchar* colnames[] = { "Name", "Size", "Date", "Permissions" };

int main (int argc, char *argv[]){

  GtkWidget *window, *hpaned, *vpaned, *dirview;
  
  gtk_init (&argc, &argv);

  //filler
  GtkWidget *button1 = gtk_button_new_with_label ("sys");
  GtkWidget *button2 = gtk_button_new_with_label ("hex");
  //end filler

  GtkListStore *dirstore;
  dirstore = gtk_list_store_new (N_COLUMNS,
                                 G_TYPE_STRING,  /* File name          */
                                 G_TYPE_STRING,  /* File size          */
                                 G_TYPE_STRING,  /* Mod Date           */
                                 G_TYPE_STRING); /* Access Permissions */



  setupWindow(&window);

  //set up panes
  hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  vpaned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);

  gtk_widget_set_size_request(hpaned, 200, 500);
  gtk_widget_set_size_request(vpaned, 500, 500);


  addDirInfoToStore(&dirstore, &dirview);
  renderDirview(&dirview);

 

  gtk_paned_pack1 (GTK_PANED (hpaned), button1, TRUE, FALSE);
  gtk_paned_pack2 (GTK_PANED (hpaned), vpaned, FALSE, FALSE);

  gtk_paned_pack1 (GTK_PANED (vpaned), dirview, TRUE, FALSE);
  gtk_paned_pack2 (GTK_PANED (vpaned), button2, TRUE, FALSE);


  gtk_container_add (GTK_CONTAINER (window), hpaned);
  gtk_widget_show_all (window);

  gtk_main ();
  return 0;
}


static void addDirInfoToStore(GtkListStore **dirstore, GtkWidget **dirview){

  int i;
  GtkTreeIter iter;

  for( i = 0; i < 2; i++){

    gtk_list_store_append (*dirstore, &iter);

    //TODO real data
    gtk_list_store_set (*dirstore, &iter,
                        NAME_COLUMN, "file1",
                        SIZE_COLUMN, "100 Kb",
                        DATE_COLUMN, "08/22/15",
                        PERM_COLUMN, "rwxr-xr--",-1);

    *dirview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (*dirstore));

  }
}

static void renderDirview(GtkWidget **dirview){

  int i;
  int j = 0;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  renderer = gtk_cell_renderer_text_new ();

  for(i = NAME_COLUMN; i <= PERM_COLUMN; i++){
    column = gtk_tree_view_column_new_with_attributes (colnames[j],
                                                       renderer,
                                                       "text",
                                                       i,
                                                       NULL);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_min_width (column, 60);
    gtk_tree_view_column_set_max_width (column, 200);

    gtk_tree_view_append_column (GTK_TREE_VIEW (*dirview), column);
    j++;
  }

}

static void setupWindow(GtkWidget **window){
  *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (*window), "File Explorer");
  gtk_container_set_border_width (GTK_CONTAINER (*window), 10);
  gtk_widget_set_size_request (*window, 700, 500);

  g_signal_connect (G_OBJECT (*window), "destroy",
                    G_CALLBACK (destroy), NULL);
}

static void destroy (GtkWidget *window, gpointer data) {
  gtk_main_quit ();
}


