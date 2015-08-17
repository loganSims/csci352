/*
  browser.c

  Logan Sims
  CSCI 352
  Assignment 3
  08/17/2015

  A file browser using GTK+3.0. The browser starts in the
  root directory. 

  IMPORTANT: A folder must be clicked on for the sub directories of that
             folder to become available. This is because the program only 
             loads the data form directories that were selected to save
             on computation time. 

*/

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_LINE 1240
#define MAX_DIRLEN 256
#define DEBUG 1

// Enum used for making directory section of browser
enum {
  DNAME = 0,
  DSIZE,
  DDATE,
  DPERM,
  UNIT,
  DICON,
  D_COLUMNS
};

// Emun used for making System section
enum {
  TNAME = 0,
  TICON,
  TPATH,
  T_COLUMNS
};

// Struct for holding the views of the
// Hex editor, used in event handler for
// directory item selection
struct HexData
{
  GtkWidget *addview;  
  GtkWidget *hexview;  
  GtkWidget *ascview; 
};

static void display (GtkWidget* hpaned);

// Directory Section
static void build_dirstore(GtkListStore *dirstore, char *path);
static void build_dirview(GtkWidget *dirview, struct HexData *hexbuffs);

// System Section
static int build_sysstore(GtkTreeStore *treestore, GtkTreeIter *iter, char *path, int is_root);
static void build_sysview(GtkWidget *sysview, GtkWidget *dirview);

// Hex editor section
int fill_hex_display(char *filepath, struct HexData *hexbuffs); 

// Event handlers
void sys_item_selected(GtkTreeSelection *selection, GtkWidget *dirview);
void dir_item_selected(GtkWidget *selection, struct HexData *hexbuffs);

// Helper
int makePerm(struct stat *sb, char *perm);

// Used for setting title of columns in directory section
const gchar *colnames[] = { "Name", "Size", "Date", "Permissions" };

/*
   Global var: w_path: working path 
   Set by sys_item_selected() and only 
   used by dir_item_selected().
   Purpose: to allow dir_item_selected to open files
            selected in the directory view.
*/
char w_path[MAX_DIRLEN];


/*
   function: main
   
   Sets up the 3 sections of the
   file browser, calls functions to
   help with set up.
*/
int main (int argc, char *argv[]){

  gtk_init (&argc, &argv);

  // Start the browser at root.
  char *start_path = "/";

  struct HexData hexinfo;
  
  GtkWidget *hpaned, *vpaned, *dirview, *hexpaned, *hexpanedr;

  // Part(a) Set up hex editor
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

  // Set up a 3 paned layout for the 3 sections of the editor
  hexpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  hexpanedr = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);

  gtk_paned_pack1 (GTK_PANED (hexpanedr), hexinfo.hexview, TRUE, FALSE);
  gtk_paned_pack2 (GTK_PANED (hexpanedr), hexinfo.ascview, TRUE, FALSE);

  gtk_paned_pack1 (GTK_PANED (hexpaned), hexinfo.addview, TRUE, FALSE);
  gtk_paned_pack2 (GTK_PANED (hexpaned), hexpanedr, TRUE, FALSE);


  // Part (b) Setup directory section.
  GtkListStore *dirstore = gtk_list_store_new(D_COLUMNS,
                                              G_TYPE_STRING, /*File name      */
                                              G_TYPE_STRING, /*File size      */
                                              G_TYPE_STRING, /*File size unit */
                                              G_TYPE_STRING, /*Mod Date       */
                                              G_TYPE_STRING, /*Access Perm    */
                                              GDK_TYPE_PIXBUF); /*Icon        */

  build_dirstore(dirstore, start_path);

  dirview = gtk_tree_view_new ();
  build_dirview(dirview, &hexinfo);

  gtk_tree_view_set_model (GTK_TREE_VIEW (dirview), 
                           GTK_TREE_MODEL (dirstore));


  // Part (c) Set up system section
  GtkTreeStore *sysstore = gtk_tree_store_new(T_COLUMNS, 
                                              G_TYPE_STRING, 
                                              GDK_TYPE_PIXBUF,
                                              G_TYPE_STRING);

  GtkTreeIter iter;

  build_sysstore(sysstore, &iter, start_path, 1);

  GtkWidget *sysview = gtk_tree_view_new();
  build_sysview(sysview, dirview);

  gtk_tree_view_set_model(GTK_TREE_VIEW(sysview), GTK_TREE_MODEL(sysstore));

  // clean up
  g_object_unref(dirstore);
  g_object_unref(sysstore);

  // Part (d) Pack it all and display.

  hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  vpaned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
  gtk_widget_set_size_request(hpaned, 800, 500);
  gtk_widget_set_size_request(vpaned, 600, 500);


  // create a scrolled window for hexpaned
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


/*
  function: build_dirstore()
  inputs: dirstore: The store that will hold the data
          path: the path to the folder whos contents are being displayed.

  Opens the directory from path and reads contents in a loop. Each pass
  of the loop formats the important information from the directory entry
  and saves it in dirstore. This function calls makePerm() to set up
  the permissions column

*/
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

        // format size and units (B or Kib)
        snprintf(size, 15, "%.1f", 
                 ((sb.st_size < 1000) ? sb.st_size : (sb.st_size/1000.0)));

        snprintf(unit, 15, "%s", 
                 ((sb.st_size < 1000) ? "B" : "KiB"));
        gtk_list_store_append (dirstore, &iter);

        // Set up icon
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
        // store it all
        gtk_list_store_set (dirstore, &iter,
                            DNAME, entry->d_name,
                            DSIZE, size,
                            UNIT, unit,
                            DDATE, modtime,
                            DPERM, permission, 
                            DICON, icon, -1);
      }
    }
  free(dp);
  }
  //clean up
  g_object_unref(dir_icon);
  g_object_unref(file_icon);
  g_object_unref(other_icon);

}

/*
  function: build_dirview()
  input: dirview: The view for the directory section.
         hexinfo: A sturct holding all 3 views for the hex editor.

  Sets up each column for rendering and adds selection handler. 
  hexinfo is passed so that the selection handling can alter the
  hex editor when a file is selected.

*/
static void build_dirview(GtkWidget *dirview, struct HexData *hexinfo){

  int i;
  int j = 0;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  // Loop through each column to be displayed
  for(i = DNAME; i <= DPERM; i++){

    column = gtk_tree_view_column_new();

    // Add icon if rendering the name column
    if (j == DNAME){
      renderer = gtk_cell_renderer_pixbuf_new();
      gtk_tree_view_column_pack_start(column, renderer, FALSE);
      gtk_tree_view_column_set_attributes(column, renderer, 
                                          "pixbuf", DICON, NULL);
    }

    renderer = gtk_cell_renderer_text_new ();

    // align the text to right if size column
    if (j == DSIZE){
       g_object_set (renderer, "xalign", 1.0, NULL);
    }

    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column, renderer, 
                                         "text", i, NULL); 
    gtk_tree_view_column_set_title(column, colnames[j]);
    gtk_tree_view_column_set_resizable (column, TRUE);

    // Set all columns but permissions to sortable
    if (j != DPERM){
      gtk_tree_view_column_set_sort_indicator(column, TRUE);           
      gtk_tree_view_column_set_sort_column_id(column, j);
    }

    // if size column add the unit as well.
    if (j == DSIZE){
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

/*
  function: build_sysstore()
  inputs: sysstore: The store for data
          iter: an iter to the directory in the systore whos children this call is adding
          path: The path to the directory whos subdirectories are being added to sysstore
          is_root: called with 1 when adding root (first call). called with 0 all other times
                   to add new data. 

  This functions adds data to the sysstore, assuming parent (iter) is already
  in the tree unless is_root is set to 1. Adds the subdirectories of the directory
  in path to the sysstore for display. The sysstore is built by only adding subdirectories
  when the parent is selected. 

  Loops thorugh contents of the directory in path, checks if they are directories,
  and adds them to sysstore if they are. Since this is called everytime a system item
  is selected it checks to make sure the selected directory hasn't already been explored.
*/
static int build_sysstore(GtkTreeStore *sysstore, GtkTreeIter *parent, char *path, int is_root){
  
  DIR *dp;
  struct stat sb;
  struct dirent *entry;
  GtkTreeIter child;

  GError *error = NULL;
  GdkPixbuf* dir_icon = gdk_pixbuf_new_from_file("icons/dir_icon.png", &error);


  if (is_root){
    gtk_tree_store_append (sysstore, parent, NULL);
    gtk_tree_store_set (sysstore, parent, 
                        TNAME, "/",
                        TICON, dir_icon, 
                        TPATH, path, -1);
  }

  // Check if iter doesn't have it's children yet
  if (gtk_tree_model_iter_has_child(GTK_TREE_MODEL(sysstore), parent) != TRUE){

    dp = opendir(path);
 
    if (dp == NULL){
      return -1;
    }

    char entrypath[MAX_DIRLEN];

    while((entry = readdir(dp))){
      if (is_root){
        snprintf(entrypath, MAX_DIRLEN, "%s%s", path, entry->d_name);
      }else{
        snprintf(entrypath, MAX_DIRLEN, "%s/%s", path, entry->d_name);
      }
      stat(entrypath, &sb);

      if(S_ISDIR(sb.st_mode)){
        if (entry->d_name[0] != '.'){   
          gtk_tree_store_append (sysstore, &child, parent);
            
          gtk_tree_store_set (sysstore, &child, 
                              TNAME, entry->d_name,
                              TICON, dir_icon,
                              TPATH, entrypath, -1);
        }
      }
    }
    closedir(dp);
  }

  g_object_unref(dir_icon);

  return 0;
}

/*
  function: build_sysview()
  inputs: sysview: The view being rendered
          dirview: The directory view passed to the selection handler
                   so that it can be changed when a directory in sysview
                   is selected. 

  Sets up the system section's view and adds selection handling.
  Needs dirview for selection handler.

*/
static void build_sysview(GtkWidget *sysview, GtkWidget *dirview){

    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    column = gtk_tree_view_column_new();

    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_set_attributes(column, renderer, 
                                        "pixbuf", TICON, NULL);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_set_attributes(column, renderer, 
                                        "text", TNAME, NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (sysview), column);

    // add selection handling
    GtkTreeSelection *selection;
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(sysview));

    // connect the selection callback function
    g_signal_connect(G_OBJECT(selection), "changed",
                     G_CALLBACK(sys_item_selected), dirview);

}

/*
  function: dir_item_selected()
  inputs: selection: the selectrion from the directory view
          hexinfo: a struct holding the hex views.

  Selection handler for the directory view. Calls fill_hex_display() with
  hexinfo to update the hex editor when an item is selected.

  Note: Reads in w_path to get the location of the file selected.

*/
void dir_item_selected (GtkWidget *selection, struct HexData *hexinfo) {

    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *name;
    char filepath[MAX_DIRLEN];    

    if (gtk_tree_selection_get_selected (GTK_TREE_SELECTION(selection), 
        &model, &iter)) {
        gtk_tree_model_get (model, &iter, DNAME, &name, -1);
        snprintf(filepath, MAX_DIRLEN, "%s%s", w_path, name);
        fill_hex_display(filepath, hexinfo); 
    }
}

/*
  function: sys_item_selected()
  inputs: selection: Selection from the sysview
          dirview: the directroy view to be updated on 
                   selection of item from sysview
  
  resets the old dirstore with new data and updates. Also updates 
  sysview by giving selected directory it's subdirectories.

  Note: updates w_path to selected directory
*/
void sys_item_selected(GtkTreeSelection *selection, GtkWidget *dirview){

  GtkTreeModel *sysmodel;
  GtkTreeModel *dirmodel;
  GtkTreeIter iter;
  GtkTreeIter child;
  gchar *dirname;
  gchar *path;


  if (gtk_tree_selection_get_selected (GTK_TREE_SELECTION(selection), 
                                       &sysmodel, &child)) {
      
    gtk_tree_model_get (sysmodel, &child, TNAME, &dirname, TPATH, &path, -1);

    snprintf(w_path, MAX_DIRLEN, "%s/", path);
    
    dirmodel = gtk_tree_view_get_model(GTK_TREE_VIEW(dirview));

    build_dirstore(GTK_LIST_STORE(dirmodel), w_path);


    gtk_tree_view_set_model (GTK_TREE_VIEW (dirview), 
                             dirmodel);

    // Update system view
    if (gtk_tree_selection_get_selected (GTK_TREE_SELECTION(selection), 
                                         &sysmodel, &iter)) {
      build_sysstore(GTK_TREE_STORE(sysmodel), &iter, path, 0);
    }  
  }
}

/*
  fill_hex_display()
  inputs: filepath: the path to the file to be opend in the hex editor.
          hexinfo: A struct holding all views of hex editor.

  Opens the file from filepath and readings in the data as Hex and Ascii, filling
  the correct buffers for the hex editor views with the data. Also keeps track of
  the address of the views in an address buffer.

*/
int fill_hex_display(char *filepath, struct HexData *hexinfo){

  int i;
  int addint = 0;
  char line[17];
  char addstr[9];
  char hexstr[4];
  GtkTextIter additer;
  GtkTextIter hexiter;  
  GtkTextIter asciter;  

  // Create the new buffers for views
  GtkTextBuffer *hexbuff = gtk_text_view_get_buffer(GTK_TEXT_VIEW(hexinfo->hexview)); 
  GtkTextBuffer *addbuff = gtk_text_view_get_buffer(GTK_TEXT_VIEW(hexinfo->addview)); 
  GtkTextBuffer *ascbuff = gtk_text_view_get_buffer(GTK_TEXT_VIEW(hexinfo->ascview)); 

  GtkTextTagTable *hextable = gtk_text_tag_table_new();
  GtkTextTagTable *addtable = gtk_text_tag_table_new();
  GtkTextTagTable *asctable = gtk_text_tag_table_new();

  hexbuff = gtk_text_buffer_new(hextable);
  addbuff = gtk_text_buffer_new(addtable);
  ascbuff = gtk_text_buffer_new(asctable);

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

  //clean up
  g_object_unref(hextable);
  g_object_unref(addtable);
  g_object_unref(asctable);
  g_object_unref(hexbuff);
  g_object_unref(addbuff);
  g_object_unref(ascbuff);
  return 0;
}

/*
   function: display()
   inputs: hpaned: the paned to put in a window

   Puts the input widget into a window with some formatting.
   Then calls gtk_widget_show_all() to display everything.
*/
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

/*
  functionL makePerm()
  inputs: sb: stat struct for the file whos permission
              are to be assembled.
          perm: string that will store human readable permissions

  A small helper function that builds file permissions
  in the rwx form.
*/
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

