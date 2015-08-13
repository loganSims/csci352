#include <gtk/gtk.h>
#include <string.h>
#include "access.h"

#define MAX_DEPTH 1000
#define MAX_DIRLEN 256

// Used for making Directory section
enum {
  NAME_COLUMN = 0,
  SIZE_COLUMN,
  DATE_COLUMN,
  PERM_COLUMN,
  ICON_COLUMN,
  N_COLUMNS
};

// Used for making System section
enum {
  TNAME_COLUMN = 0,
  TICON_COLUMN,
  TLEVEL,
  T_COLUMNS
};

struct HexData
{
  GtkTextBuffer* addbuff;  
  GtkTextBuffer* hexbuff;  
  GtkTextBuffer* ascbuff; 
};

static void display (GtkWidget* hpaned);

static void build_dirstore(GtkListStore *dirstore, char *path);
static void build_dirview(GtkWidget *dirview, struct HexData *hexbuffs);

static int build_sysstore(GtkTreeStore *treestore, GtkTreeIter *iter, char *path, int is_root);

static void build_sysview(GtkWidget *sysview, GtkWidget *dirview);

void sys_item_selected(GtkTreeSelection *selection, GtkWidget *dirview);

void dir_item_selected(GtkWidget *selection, struct HexData *hexbuffs);

int fill_hex_display(char *filepath, struct HexData *hexbuffs); 

const gchar *colnames[] = { "Name", "Size", "Date", "Permissions" };


/*
   Global working path, only set by sys_item_selected and only 
   used by dir_item_selected.
   Purpose: to allow dir_item_selected to open files
            selected in the directory view.
*/
char w_path[MAX_DIRLEN];

int main (int argc, char *argv[]){

  gtk_init (&argc, &argv);

  char *start_path = "/";

  struct HexData hexbuffs;
  
  GtkWidget *hpaned, *vpaned, *dirview, *hexbox;

  GtkWidget *addview = gtk_text_view_new();
  GtkWidget *hexview = gtk_text_view_new();
  GtkWidget *ascview = gtk_text_view_new();

  hexbuffs.addbuff = gtk_text_view_get_buffer(GTK_TEXT_VIEW (addview));  
  hexbuffs.hexbuff = gtk_text_view_get_buffer(GTK_TEXT_VIEW (hexview));  
  hexbuffs.ascbuff = gtk_text_view_get_buffer(GTK_TEXT_VIEW (ascview));  

  // Fill hexbox with textviews

  hexbox = gtk_box_new (FALSE, 5);

  gtk_box_pack_start (GTK_BOX (hexbox), addview, FALSE, FALSE, 5);
  gtk_box_pack_start (GTK_BOX (hexbox), hexview, FALSE, FALSE, 5);
  gtk_box_pack_start (GTK_BOX (hexbox), ascview, FALSE, FALSE, 5);


  // setup dirstore
  GtkListStore *dirstore = gtk_list_store_new(N_COLUMNS,
                                              G_TYPE_STRING, /*File name   */
                                              G_TYPE_STRING, /*File size   */
                                              G_TYPE_STRING, /*Mod Date    */
                                              G_TYPE_STRING, /*Access Perm */
                                              GDK_TYPE_PIXBUF); /*Icon     */

  build_dirstore(dirstore, start_path);

  // setup dirview
  dirview = gtk_tree_view_new ();
  build_dirview(dirview, &hexbuffs);

  //connect dirview and dirstore
  gtk_tree_view_set_model (GTK_TREE_VIEW (dirview), 
                           GTK_TREE_MODEL (dirstore));


  // setup sysstore
  GtkTreeStore *sysstore = gtk_tree_store_new(T_COLUMNS, 
                                              G_TYPE_STRING, 
                                              GDK_TYPE_PIXBUF,
                                              G_TYPE_INT);

  GtkTreeIter iter;

  build_sysstore(sysstore, &iter, start_path, 1);

  // setup sysview
  GtkWidget *sysview = gtk_tree_view_new();
  build_sysview(sysview, dirview);

  // connect sysview adn sysstore
  gtk_tree_view_set_model(GTK_TREE_VIEW(sysview), GTK_TREE_MODEL(sysstore));


  g_object_unref(dirstore);
  g_object_unref(sysstore);

  //setup panes & pack them
  hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  vpaned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
  gtk_widget_set_size_request(hpaned, 200, 500);
  gtk_widget_set_size_request(vpaned, 500, 500);


  // create a scrolled window for hexbox
  GtkWidget* hex_scroller = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (hex_scroller),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);

  gtk_container_add (GTK_CONTAINER (hex_scroller), hexbox);

  // create a scrolled window for dirview
  GtkWidget* dir_scroller = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (dir_scroller),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);

  gtk_container_add (GTK_CONTAINER (dir_scroller), dirview);



  // create a scrolled window for sysview
  GtkWidget* sys_scroller = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sys_scroller),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);

  gtk_container_add (GTK_CONTAINER (sys_scroller), sysview);


  // pack vpaned
  gtk_paned_pack1 (GTK_PANED (vpaned), dir_scroller, TRUE, FALSE);
  gtk_paned_pack2 (GTK_PANED (vpaned), hex_scroller, TRUE, FALSE);

  // pack hpaned
  gtk_paned_pack1 (GTK_PANED (hpaned), sys_scroller, TRUE, FALSE);
  gtk_paned_pack2 (GTK_PANED (hpaned), vpaned, FALSE, FALSE);



  display(hpaned);

  gtk_main ();
  return 0;
}

static void build_dirstore(GtkListStore *dirstore, char *path){

  GtkTreeIter iter;
  DIR *dp;
  struct stat sb;
  struct dirent *entry;
  struct tm time;
  char entrypath[MAX_DIRLEN];
  char modtime[9];
  char permission[10];
  char size[15];
  GError *error = NULL;
  GdkPixbuf* dir_icon = gdk_pixbuf_new_from_file("dir_icon.png", &error);
  GdkPixbuf* file_icon = gdk_pixbuf_new_from_file("file_icon.png", &error);
  GdkPixbuf* other_icon = gdk_pixbuf_new_from_file("icon2.png", &error);
  GdkPixbuf* icon;

  dp = opendir(path);

  if (dp != NULL){

    while((entry = readdir(dp))){

      if (entry->d_name[0] != '.'){   

        // Get path of entry for stat
        snprintf(entrypath, MAX_DIRLEN, "%s%s", path, entry->d_name);
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
          default:  icon = other_icon; break; //other
        }

        gtk_list_store_set (dirstore, &iter,
                            NAME_COLUMN, entry->d_name,
                            SIZE_COLUMN, size,
                            DATE_COLUMN, modtime,
                            PERM_COLUMN, permission, 
                            ICON_COLUMN, icon, -1);

      }
    }
  }
}

static void build_dirview(GtkWidget *dirview, struct HexData *hexbuffs){

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
                    G_CALLBACK(dir_item_selected), hexbuffs);

}


//iter is already in tree, adding its children
static int build_sysstore(GtkTreeStore *sysstore, GtkTreeIter *iter, char *path, int is_root){
  
  DIR *dp;
  struct stat sb;
  struct dirent *entry;

  GtkTreeIter child;

  GError *error = NULL;
  GdkPixbuf* dir_icon = gdk_pixbuf_new_from_file("dir_icon.png", &error);


  if (is_root){
    gtk_tree_store_append (sysstore, iter, NULL);
    gtk_tree_store_set (sysstore, iter, 
                        TNAME_COLUMN, "/",
                        TICON_COLUMN, dir_icon, -1);
  }

  // Check if iter hasn't gotten it's children yet
  if (gtk_tree_model_iter_has_child(GTK_TREE_MODEL(sysstore), iter) != TRUE){

    dp = opendir(path);
 
    if (dp == NULL){
      return -1;
    }

    char entrypath[MAX_DIRLEN];

    while((entry = readdir(dp))){
      snprintf(entrypath, MAX_DIRLEN, "%s%s", path, entry->d_name);
      stat(entrypath, &sb);
      if(S_ISDIR(sb.st_mode)){
        if (entry->d_name[0] != '.'){   
          gtk_tree_store_append (sysstore, &child, iter);
            
          gtk_tree_store_set (sysstore, &child, 
                              TNAME_COLUMN, entry->d_name,
                              TICON_COLUMN, dir_icon, -1);
        }
      }
    }
    closedir(dp);
  }

  return 0;
}
static void build_sysview(GtkWidget *sysview, GtkWidget *dirview){

    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    // add the Position to the treeview
    column = gtk_tree_view_column_new();

    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_set_attributes(column, renderer, 
                                        "pixbuf", TICON_COLUMN, NULL);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_set_attributes(column, renderer, 
                                        "text", TNAME_COLUMN, NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (sysview), column);

    // add selection handling
    GtkTreeSelection *selection;
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(sysview));

    // connect the selection callback function
    g_signal_connect(G_OBJECT(selection), "changed",
                     G_CALLBACK(sys_item_selected), dirview);

}



// Start hex viewer
void dir_item_selected (GtkWidget *selection, struct HexData *hexbuffs) {

    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *name;
    char filepath[MAX_DIRLEN];    

    if (gtk_tree_selection_get_selected (GTK_TREE_SELECTION(selection), 
        &model, &iter)) {
        
        gtk_tree_model_get (model, &iter, NAME_COLUMN, &name, -1);

        snprintf(filepath, MAX_DIRLEN, "%s%s", w_path, name);

        g_message("selected %s\n", filepath);
        //fill_hex_display(filepath, hexbuffs); 

    }
}


void sys_item_selected(GtkTreeSelection *selection, GtkWidget *dirview){

  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkTreeIter child;
  GtkTreeIter parent;
  gchar *dirname;
  char path[MAX_DIRLEN];
  char newpath[MAX_DIRLEN];

  // Part (a) build path

  if (gtk_tree_selection_get_selected (GTK_TREE_SELECTION(selection), 
                                       &model, &child)) {
      
    gtk_tree_model_get (model, &child, TNAME_COLUMN, &dirname, -1);

    if(strcmp(dirname, "/") == 0){ 
      snprintf(path, MAX_DIRLEN, "%s", dirname);
    }else{
      snprintf(path, MAX_DIRLEN, "%s/", dirname);
    } 
    while (gtk_tree_model_iter_parent(model, &parent, &child) != FALSE){

      gtk_tree_model_get(model, &parent, TNAME_COLUMN, &dirname, -1);

      if(strcmp(dirname, "/") == 0){ 
        snprintf(newpath, MAX_DIRLEN, "%s%s", dirname, path);
      }else{
        snprintf(newpath, MAX_DIRLEN, "%s/%s", dirname, path);
      }
      strcpy(path, newpath);
      newpath[0] = '\0';
      child = parent;
    }

    g_message("%s\n", path);
    // Set global cwd to path
    strcpy(w_path, path);

    // Part (b) build new dir store

    GtkListStore *dirstore = gtk_list_store_new(N_COLUMNS,
                                                G_TYPE_STRING, /*File name   */
                                                G_TYPE_STRING, /*File size   */
                                                G_TYPE_STRING, /*Mod Date    */
                                                G_TYPE_STRING, /*Access Perm */
                                                GDK_TYPE_PIXBUF); /*Icon     */
    build_dirstore(dirstore, path);

    //connect dirview and dirstore
    gtk_tree_view_set_model (GTK_TREE_VIEW (dirview), 
                             GTK_TREE_MODEL (dirstore));

    g_object_unref(dirstore);

    // Part (c) update sys tree 
    if (gtk_tree_selection_get_selected (GTK_TREE_SELECTION(selection), 
                                         &model, &iter)) {
      build_sysstore(GTK_TREE_STORE(model), &iter, path, 0);
    }  
  }
}


int fill_hex_display(char *filepath, struct HexData *hexbuffs){



  return 0;
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


