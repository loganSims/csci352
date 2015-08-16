#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_LINE 1240
#define MAX_DIRLEN 256
#define DEBUG 1

// Used for making Directory section
enum {
  NAME_COLUMN = 0,
  SIZE_COLUMN,
  DATE_COLUMN,
  PERM_COLUMN,
  UNIT,
  ICON_COLUMN,
  N_COLUMNS
};

// Used for making System section
enum {
  TNAME_COLUMN = 0,
  TICON_COLUMN,
  TPATH,
  T_COLUMNS
};

struct HexData
{
  GtkWidget *addview;  
  GtkWidget *hexview;  
  GtkWidget *ascview; 
};

static void display (GtkWidget* hpaned);

static void build_dirstore(GtkListStore *dirstore, char *path);
static void build_dirview(GtkWidget *dirview, struct HexData *hexbuffs);

static int build_sysstore(GtkTreeStore *treestore, GtkTreeIter *iter, char *path, int is_root);

static void build_sysview(GtkWidget *sysview, GtkWidget *dirview);

void sys_item_selected(GtkTreeSelection *selection, GtkWidget *dirview);

void dir_item_selected(GtkWidget *selection, struct HexData *hexbuffs);

int fill_hex_display(char *filepath, struct HexData *hexbuffs); 

int makePerm(struct stat *sb, char *perm);

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

  struct HexData hexinfo;
  
  GtkWidget *hpaned, *vpaned, *dirview, *hexpaned, *hexpanedr;

  GtkWidget *addview = gtk_text_view_new();
  GtkWidget *hexview = gtk_text_view_new();
  GtkWidget *ascview = gtk_text_view_new();
  gtk_widget_set_size_request(addview, 70, 225);
  gtk_widget_set_size_request(hexview, 300, 225);
  gtk_widget_set_size_request(ascview, 125, 225);

  gtk_text_view_set_editable(GTK_TEXT_VIEW(addview),FALSE);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(hexview),FALSE);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(ascview),FALSE);

  hexinfo.addview = addview;
  hexinfo.hexview = hexview;
  hexinfo.ascview = ascview;


  // Fill hexbox with textviews
  hexpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  hexpanedr = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);

  gtk_paned_pack1 (GTK_PANED (hexpanedr), hexinfo.hexview, TRUE, FALSE);
  gtk_paned_pack2 (GTK_PANED (hexpanedr), hexinfo.ascview, TRUE, FALSE);

  gtk_paned_pack1 (GTK_PANED (hexpaned), hexinfo.addview, TRUE, FALSE);
  gtk_paned_pack2 (GTK_PANED (hexpaned), hexpanedr, TRUE, FALSE);


  // setup dirstore
  GtkListStore *dirstore = gtk_list_store_new(N_COLUMNS,
                                              G_TYPE_STRING, /*File name      */
                                              G_TYPE_STRING, /*File size      */
                                              G_TYPE_STRING, /*File size unit */
                                              G_TYPE_STRING, /*Mod Date       */
                                              G_TYPE_STRING, /*Access Perm    */
                                              GDK_TYPE_PIXBUF); /*Icon        */

  build_dirstore(dirstore, start_path);

  // setup dirview
  dirview = gtk_tree_view_new ();
  build_dirview(dirview, &hexinfo);

  //connect dirview and dirstore
  gtk_tree_view_set_model (GTK_TREE_VIEW (dirview), 
                           GTK_TREE_MODEL (dirstore));


  // setup sysstore
  GtkTreeStore *sysstore = gtk_tree_store_new(T_COLUMNS, 
                                              G_TYPE_STRING, 
                                              GDK_TYPE_PIXBUF,
                                              G_TYPE_STRING);

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
  gtk_widget_set_size_request(hpaned, 800, 500);
  gtk_widget_set_size_request(vpaned, 600, 500);


  // create a scrolled window for hexbox
  GtkWidget* hex_scroller = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (hex_scroller),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);

  gtk_container_add (GTK_CONTAINER (hex_scroller), hexpaned);

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
  char unit[5];
  int i;
  int exe;
  GError *error = NULL;
  GdkPixbuf* dir_icon = gdk_pixbuf_new_from_file("icons/dir_icon.png", &error);
  GdkPixbuf* file_icon = gdk_pixbuf_new_from_file("icons/file_icon.png", &error);
  GdkPixbuf* exe_icon = gdk_pixbuf_new_from_file("icons/exe_icon.png", &error);
  GdkPixbuf* other_icon = gdk_pixbuf_new_from_file("icons/other_icon.png", &error);
  GdkPixbuf* icon;

  gtk_list_store_clear(dirstore);

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
        snprintf(size, 15, "%.1f", 
                 ((sb.st_size < 1000) ? sb.st_size : (sb.st_size/1000.0)));

        snprintf(unit, 15, "%s", 
                 ((sb.st_size < 1000) ? "B" : "KiB"));
        gtk_list_store_append (dirstore, &iter);

        // setup icon  TODO c file, exe file, ...
        switch (sb.st_mode & S_IFMT) {
          case S_IFDIR: icon = dir_icon; break;
          case S_IFREG: 
            exe = 1;
            for (i = 0; entry->d_name[i]; i++){
              if(entry->d_name[i] == '.'){
                exe = 0;
              }
            }
            if (exe){
              icon = exe_icon;
            }else{
              icon = file_icon; 
            }
            break;
          default:  icon = other_icon; break; //other
        }





        gtk_list_store_set (dirstore, &iter,
                            NAME_COLUMN, entry->d_name,
                            SIZE_COLUMN, size,
                            UNIT, unit,
                            DATE_COLUMN, modtime,
                            PERM_COLUMN, permission, 
                            ICON_COLUMN, icon, -1);
      }
    }
  free(dp);
  }
  g_object_unref(dir_icon);
  g_object_unref(file_icon);
  g_object_unref(other_icon);

}

static void build_dirview(GtkWidget *dirview, struct HexData *hexinfo){

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

    if (j != PERM_COLUMN){
      gtk_tree_view_column_set_sort_indicator(column, TRUE);           
      gtk_tree_view_column_set_sort_column_id(column, j);
    }

    if (j == SIZE_COLUMN){
      renderer = gtk_cell_renderer_text_new ();
      gtk_tree_view_column_pack_start(column, renderer, FALSE);
      gtk_tree_view_column_set_attributes(column, renderer, 
                                          "text", UNIT, NULL);
    }

    gtk_tree_view_append_column (GTK_TREE_VIEW (dirview), column);
    j++;
  }

  // add selection handling
  GtkTreeSelection *selection;
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(dirview));
    
  // connect the selection callback function
  g_signal_connect (G_OBJECT(selection), "changed", 
                    G_CALLBACK(dir_item_selected), hexinfo);

}


//iter is already in tree, adding its children
static int build_sysstore(GtkTreeStore *sysstore, GtkTreeIter *iter, char *path, int is_root){
  
  DIR *dp;
  struct stat sb;
  struct dirent *entry;

  GtkTreeIter child;

  GError *error = NULL;
  GdkPixbuf* dir_icon = gdk_pixbuf_new_from_file("icons/dir_icon.png", &error);


  if (is_root){
    gtk_tree_store_append (sysstore, iter, NULL);
    gtk_tree_store_set (sysstore, iter, 
                        TNAME_COLUMN, "/",
                        TICON_COLUMN, dir_icon, 
                        TPATH, path, -1);
  }

  // Check if iter hasn't gotten it's children yet
  if (gtk_tree_model_iter_has_child(GTK_TREE_MODEL(sysstore), iter) != TRUE){

    dp = opendir(path);
 
    if (dp == NULL){
      return -1;
    }

    char entrypath[MAX_DIRLEN];

    while((entry = readdir(dp))){
      snprintf(entrypath, MAX_DIRLEN, "%s%s/", path, entry->d_name);
      stat(entrypath, &sb);
      if(S_ISDIR(sb.st_mode)){
        if (entry->d_name[0] != '.'){   
          gtk_tree_store_append (sysstore, &child, iter);
            
          gtk_tree_store_set (sysstore, &child, 
                              TNAME_COLUMN, entry->d_name,
                              TICON_COLUMN, dir_icon,
                              TPATH, entrypath, -1);
        }
      }
    }
    closedir(dp);
  }

  g_object_unref(dir_icon);

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
void dir_item_selected (GtkWidget *selection, struct HexData *hexinfo) {

    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *name;
    char filepath[MAX_DIRLEN];    

    if (gtk_tree_selection_get_selected (GTK_TREE_SELECTION(selection), 
        &model, &iter)) {
        
        gtk_tree_model_get (model, &iter, NAME_COLUMN, &name, -1);

        snprintf(filepath, MAX_DIRLEN, "%s%s", w_path, name);

        g_message("selected %s\n", filepath);
        fill_hex_display(filepath, hexinfo); 

    }
}


void sys_item_selected(GtkTreeSelection *selection, GtkWidget *dirview){

  GtkTreeModel *sysmodel;
  GtkTreeModel *dirmodel;
  GtkTreeIter iter;
  GtkTreeIter child;
  gchar *dirname;
  gchar *path;

  // Part (a) build path

  if (gtk_tree_selection_get_selected (GTK_TREE_SELECTION(selection), 
                                       &sysmodel, &child)) {
      
    gtk_tree_model_get (sysmodel, &child, TNAME_COLUMN, &dirname, TPATH, &path, -1);

    g_message("%s\n", path);
    // Set global cwd to path
    strcpy(w_path, path);

    // Part (b) get model and update with new data
    dirmodel = gtk_tree_view_get_model(GTK_TREE_VIEW(dirview));

    build_dirstore(GTK_LIST_STORE(dirmodel), path);

    //connect dirview and dirstore

    gtk_tree_view_set_model (GTK_TREE_VIEW (dirview), 
                             dirmodel);


    // Part (c) update sys tree 
    if (gtk_tree_selection_get_selected (GTK_TREE_SELECTION(selection), 
                                         &sysmodel, &iter)) {
      build_sysstore(GTK_TREE_STORE(sysmodel), &iter, path, 0);
    }  
  }
}


int fill_hex_display(char *filepath, struct HexData *hexinfo){

  int i;
  int addint = 0;
  char line[17];
  char addstr[9];
  char hexstr[4];
  GtkTextIter additer;
  GtkTextIter hexiter;  
  GtkTextIter asciter;  

  GtkTextBuffer *hexbuff = gtk_text_view_get_buffer(GTK_TEXT_VIEW(hexinfo->hexview)); 
  GtkTextBuffer *addbuff = gtk_text_view_get_buffer(GTK_TEXT_VIEW(hexinfo->addview)); 
  GtkTextBuffer *ascbuff = gtk_text_view_get_buffer(GTK_TEXT_VIEW(hexinfo->ascview)); 

  GtkTextTagTable *hextable = gtk_text_tag_table_new();
  GtkTextTagTable *addtable = gtk_text_tag_table_new();
  GtkTextTagTable *asctable = gtk_text_tag_table_new();


  hexbuff = gtk_text_buffer_new(hextable);
  addbuff = gtk_text_buffer_new(addtable);
  ascbuff = gtk_text_buffer_new(asctable);

  g_object_unref(hextable);
  g_object_unref(addtable);
  g_object_unref(asctable);

  int input;
    
  if ((input = open(filepath, O_RDONLY)) < 0) {
      printf("File %s not found\n", filepath);
      return 1;
  }


  // initialize the buffer's iterator
  gtk_text_buffer_get_iter_at_offset(addbuff, &additer, 0);
  gtk_text_buffer_get_iter_at_offset(hexbuff, &hexiter, 0);
  gtk_text_buffer_get_iter_at_offset(ascbuff, &asciter, 0);
    
  // add each line from the file to the buffer
  while (read(input, line, 16) > 0){ 
     snprintf(addstr, 9, "%08d", addint);
     addstr[8] = '\0';
     gtk_text_buffer_insert (addbuff, &additer, addstr, -1);
     gtk_text_buffer_insert (addbuff, &additer, "\n", -1);

     //remove unprintable for ascii column
     for(i = 0; i < 16; i++){
       
       snprintf(hexstr, 4, " %02x", line[i]);
       hexstr[3] = '\0';
       gtk_text_buffer_insert (hexbuff, &hexiter, hexstr, -1);

       if(line[i] < 32 || line[i] > 126){
         line[i] = '.';
       }  
     }
     line[16] = '\0';
     gtk_text_buffer_insert (ascbuff, &asciter, line, -1);
     gtk_text_buffer_insert (ascbuff, &asciter, "\n", -1);
     gtk_text_buffer_insert (hexbuff, &hexiter, "\n", -1);
     addint += 16;

     gtk_text_view_set_buffer(GTK_TEXT_VIEW(hexinfo->hexview), hexbuff);
     gtk_text_view_set_buffer(GTK_TEXT_VIEW(hexinfo->addview), addbuff);
     gtk_text_view_set_buffer(GTK_TEXT_VIEW(hexinfo->ascview), ascbuff);

     memset(line, 0, sizeof(line));
     memset(addstr, 0, sizeof(addstr));
   
  }
  g_object_unref(hexbuff);
  g_object_unref(addbuff);
  g_object_unref(ascbuff);
  return 0;
}

void display (GtkWidget *hpaned) {
    
  // create the window
  GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "File Explorer");
  gtk_container_set_border_width (GTK_CONTAINER (window), 10);
  gtk_widget_set_size_request (window, 800, 500);
  g_signal_connect (window, "delete_event", gtk_main_quit, NULL);
    
  // pack the containers
  gtk_container_add (GTK_CONTAINER (window), hpaned);
  gtk_widget_show_all (window);
}

int makePerm(struct stat *sb, char *perm){

  // User
  perm[0] = (sb->st_mode & S_IRUSR) ? 'r' : '-';
  perm[1] = (sb->st_mode & S_IWUSR) ? 'w' : '-';
  perm[2] = (sb->st_mode & S_IXUSR) ? 'x' : '-';

  // Group
  perm[3] = (sb->st_mode & S_IRGRP) ? 'r' : '-';
  perm[4] = (sb->st_mode & S_IWGRP) ? 'w' : '-';
  perm[5] = (sb->st_mode & S_IXGRP) ? 'x' : '-';

  // Other 
  perm[6] = (sb->st_mode & S_IROTH) ? 'r' : '-';
  perm[7] = (sb->st_mode & S_IWOTH) ? 'w' : '-';
  perm[8] = (sb->st_mode & S_IXOTH) ? 'x' : '-';

  perm[9] = '\0';

  return 0;
}

