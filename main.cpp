/* Bismillohir Rahmoniyr Rahiym
*/

// Muqobil Dasturlar To'plami (c) hijriy 1435-1436

#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <ctime>

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <sqlite3.h>
//#include <mysql/mysql.h>

#include <Excel.h>
#include <SqliteExport.h>

using namespace std;

#define WORD_SIZE 64

typedef struct _TreeItem TreeItem;

/* In points */
#define HEADER_HEIGHT (10*72/25.4)
#define HEADER_GAP (3*72/25.4)

typedef struct _PrintData PrintData;

struct _PrintData
{
  gchar *resourcename;
  gdouble font_size;

  gint lines_per_page;
  gchar **lines;
  gint num_lines;
  gint num_pages;
};


struct _TreeItem
{
  gchar *product;
  gchar *factProdID;
  gint   number;
  gchar *prop;
  gint   quant;
  gfloat price;
  gfloat factPrice;
  gboolean tick;
  gint  miqdor;
  //TreeItem *children;
};

//Item tmp_pr;//used in here and combo entry to determain the Item

/*bool operator==(const Item &x, const Item &y)
{
    bool ret = false;
    //cout << x.product << endl;

    if( g_strcmp0(x.product, tmp_pr.product)== 0 )
    {
        g_strcmp0(y.prop, x.prop) == 0 ? ret = true : ret = false;
        //if(ret)   cout << x.prop << " " << y.prop << endl;
    }
    return ret;
    //return y.prop == x.prop;
}

bool operator<(const Item &x, const Item &y)
{
    return x.prop < y.prop;
}*/


enum
{
  COLUMN_ITEM_PRODUCT=0,
  COLUMN_ITEM_NUMBER,
  COLUMN_ITEM_FACT_ID,
  COLUMN_ITEM_PROP,
  COLUMN_ITEM_QUANT,
  COLUMN_ITEM_PRICE,
  COLUMN_ITEM_FACT_PRICE,
  COLUMN_ITEM_PROD_TICK,
  COLUMN_ITEM_MIQDOR,

  VISIBLE_COLUMN,
  //WORLD_COLUMN,
  NUM_ITEM_COLUMNS
};

enum
{
  COLUMN_PROP_COMBO=0,
  NUM_NUMBER_COLUMNS
};
//consts
const char ver[]      = { "0.73" };
const char prog_name[]= {"GDo'kon"};

static GtkWidget *win = NULL;

GtkWidget *treeview = NULL, *db_del_but, *db_ins_but, *db_new_but, *db_export_but;
gchar *gPath_str;

GtkCellRenderer   *arenderer[6];

GtkTreeSelection *tree_select;

GtkTreeModel *items_model = NULL;
GtkTreeModel *numbers_model = NULL;

static GArray *articles = NULL;
static gpointer foreach_data = NULL;


struct tm *pTime;
string gTimeStr;

vector<TreeItem> prods;
map<string, vector<TreeItem> > map_prods;
//map<string, vector<string> > map_prods;
bool admin_mode = false, bsave_db;
int res_query;
string query = "select * from products", update;

/*MYSQL db;//, *pdb;
MYSQL_RES *mRes=NULL;
MYSQL_FIELD *mFld;
MYSQL_ROWS mRows;*/
char **row; //db rows
gint rows=0;

sqlite3 *pDB, *pProdsBoughtDB;
sqlite3_stmt *pStmt;

int InitDB();
static GtkTreeModel *  create_items_model (void);
static GtkTreeModel * create_numbers_model(void);

static void fill_prodcmb_entry (GtkWidget *combo);
static void add_items(void);
static void add_columns (GtkTreeView  *treeview/*, GtkTreeModel *items_model, GtkTreeModel *numbers_model*/);
static void item_toggled (GtkCellRendererToggle *cell, gchar *path_str, gpointer  data);
static void toggle_admin (GtkToggleButton *check_button,   GtkSizeGroup    *size_group);

static void cell_edited (GtkCellRendererText *cell,
             const gchar         *path_string,
             const gchar         *new_text,
             gpointer             data);

static void selected_foreach( GtkTreeModel *items_model,
                              GtkTreePath  *path,
                              GtkTreeIter  *iter,
                              gpointer     data );

void MessageBox( string &msg, GtkMessageType msgtype = GTK_MESSAGE_INFO);
void about_us( GtkWidget *widget,   gpointer   data );

static void
begin_print (GtkPrintOperation *operation,
             GtkPrintContext   *context,
             gpointer           user_data);
static void draw_page (GtkPrintOperation *operation,
           GtkPrintContext   *context,
           gint               page_nr,
           gpointer           user_data);
static void end_print (GtkPrintOperation *operation,
           GtkPrintContext   *context,
           gpointer           user_data);

GtkWidget * do_printing (GtkWidget *do_widget);

/*static void tree_select_cb(GtkTreeSelection *sel, gpointer data)
{
   GtkTreeModel *items_model, *prop_model;
   GtkTreePath  *path;
   GtkTreeIter  iter;
   gchar *name;

   prop_model = (GtkTreeModel*)data;

   if(gtk_tree_selection_get_selected(sel, &items_model, &iter))
   {
    gtk_tree_model_get (items_model, &iter, COLUMN_ITEM_PRODUCT, &name, -1);

       string str(name);
       vector<string> vprods = map_prods[str];

       gtk_list_store_clear(GTK_LIST_STORE(prop_model));
       tmp_pr.product = g_strdup(name);
       //printf("%s\n", name);
       //gtk_tree_model_get_iter_first(prop_model, &iter);

       //gtk_tree_model_get_iter_from_string(prop_model, &iter, "0");
       if( vprods.size() >= 1 )
       {

            for(int i=0; i < vprods.size(); i++)
            {
            //gtk_list_store_remove(GTK_LIST_STORE(prop_model), &iter);
                //cout << vprods[i] << " ";

                gtk_list_store_append(GTK_LIST_STORE(prop_model), &iter);
                gtk_list_store_set(GTK_LIST_STORE(prop_model), &iter, COLUMN_PROP_COMBO, reinterpret_cast<gchar*>(&vprods[i][0]), -1);
            }
        //endl(cout);
      }
     //cout << name << endl;
    g_free(name);
   }

}*/

/*static void miqdor_kbpress_cb (GtkWidget *widget,   GtkWidget *dlg)
{
    g_signal_emit_by_name(G_OBJECT(dlg), "response", GTK_RESPONSE_ACCEPT);
}*/
void  price_cell_data_func (GtkTreeViewColumn *col,
                           GtkCellRenderer   *renderer,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           user_data)
{
     gfloat  price;
     gchar   buf[20];
     //gint *_case = (gint*)(user_data);

     //gtk_tree_view_get_cursor(GTK_TREE_VIEW(text_view), &path, &column);
     //cout << *_case << endl;
     gtk_tree_model_get(model, iter, COLUMN_ITEM_PRICE, &price, -1);
     g_object_set(renderer, "foreground", "Red", "foreground-set", TRUE, NULL);

     g_snprintf(buf, sizeof(buf), "%.1f", price);

     g_object_set(renderer, "text", buf, NULL);
}

void  factPrice_cell_data_func (GtkTreeViewColumn *col,
                           GtkCellRenderer   *renderer,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           user_data)
{
     gfloat  price;
     gchar   buf[20];
     //gint *_case = (gint*)(user_data);

     //gtk_tree_view_get_cursor(GTK_TREE_VIEW(text_view), &path, &column);
     //cout << *_case << endl;
     gtk_tree_model_get(model, iter, COLUMN_ITEM_FACT_PRICE, &price, -1);
     //g_object_set(renderer, "foreground", "Red", "foreground-set", TRUE, NULL);

     g_snprintf(buf, sizeof(buf), "%.1f", price);

     g_object_set(renderer, "text", buf, NULL);
}

static void enter_miqdor( gint *data)
{
  GtkWidget *content_area;
  GtkWidget *dialog;
  GtkWidget *hbox;
  GtkWidget *stock;
  GtkWidget *table;
  GtkWidget *local_entry1;
  GtkWidget *local_entry2;
  GtkWidget *label;
  gint response, miqdor;
  guint event_handler_id;
  const gchar *num_prod;

  dialog = gtk_dialog_new_with_buttons ("Enter Quantity:",
                                        GTK_WINDOW (win),
                                        GTK_DIALOG_DESTROY_WITH_PARENT, //| GTK_DIALOG_MODAL,
                                        GTK_STOCK_OK,
                                        GTK_RESPONSE_OK,
                                        NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 8);
  gtk_box_pack_start (GTK_BOX (content_area), hbox, FALSE, FALSE, 0);

  /*stock = gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start (GTK_BOX (hbox), stock, FALSE, FALSE, 0);*/
  local_entry1 = gtk_entry_new ();

  //gtk_label_set_mnemonic_widget (GTK_LABEL (label), local_entry1);
  gtk_box_pack_start (GTK_BOX (hbox), local_entry1, FALSE, FALSE, 0);
  gtk_entry_set_activates_default(GTK_ENTRY(local_entry1), TRUE);

  /*event_handler_id = g_signal_connect(G_OBJECT(local_entry1), "activate",
                                        G_CALLBACK(miqdor_kbpress_cb), (gpointer)dialog );*/

  gtk_widget_show_all (hbox);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  if (response == GTK_RESPONSE_OK)
  {
    num_prod = gtk_entry_get_text(GTK_ENTRY(local_entry1));
    //g_print(num_prod);
    miqdor = strtod(num_prod, NULL);

    if(data != NULL) (*data) = miqdor;
    //memset(data, miqdor, sizeof miqdor) ;
  }

  gtk_widget_destroy (dialog);
}

static gboolean
key_press_cb (GtkWidget *text_view,
                 GdkEventKey *event)
{
  GtkTreeIter iter, parent_iter;
  GtkTextBuffer *buffer;
  GtkTreePath *path;
  GtkTreeViewColumn *column;
  gint n=0, i, x, prodID;


    gtk_tree_view_get_cursor(GTK_TREE_VIEW(text_view), &path, &column);
    if(column != NULL) x = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (column), "column"));

    if((event->keyval >= GDK_KEY_0) && (event->keyval <= GDK_KEY_9) )
    {
        gtk_tree_view_get_cursor(GTK_TREE_VIEW(text_view), &path, &column);
        gtk_tree_view_set_cursor(GTK_TREE_VIEW(text_view), path, column, TRUE);
        gdk_event_put( (GdkEvent*)event );
    }
    if(event->keyval == GDK_KEY_Return && x < 7 && !admin_mode)
    {
        string str;

        //gtk_tree_view_get_cursor(GTK_TREE_VIEW(text_view), &path, &column);
        //cout << gtk_tree_view_column_get_x_offset(column) << endl;
        //column = gtk_tree_view_get_column(GTK_TREE_VIEW(text_view), 3);
        if(column != NULL) x = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (column), "column"));
        //gtk_tree_view_column_cell_get_position(column, arenderer[0], &x, NULL);

        gtk_tree_model_get_iter(items_model, &iter, path);
        gtk_tree_model_get (items_model, &iter, COLUMN_ITEM_NUMBER, &prodID, -1);


        if(prodID != 0){
        gchar *prodName=new gchar[WORD_SIZE];


        gtk_tree_model_iter_parent(items_model, &parent_iter, &iter);
        gtk_tree_model_get(items_model, &parent_iter, COLUMN_ITEM_PRODUCT, &prodName, -1);

        str.assign(prodName);

            i=0;
            while(map_prods[str][i].number != prodID)
            {
                i++;
            }

         delete [] prodName;

         enter_miqdor(&n);
         /*if( n > 0){
            if(map_prods[str][i].miqdor >= 0){
                map_prods[str][i].miqdor +=n;
                map_prods[str][i].tick = true;
            }
            else{
                map_prods[str][i].miqdor =n;
                map_prods[str][i].tick = false;
            }
        }
        else if(n==0){
            map_prods[str][i].miqdor = 0;
            map_prods[str][i].tick = false;
        }*/
         map_prods[str][i].miqdor =n;
         map_prods[str][i].miqdor > 0 ? map_prods[str][i].tick = true  :
                                                                 map_prods[str][i].tick = false;
         //if(n >= 0)
            {
                gtk_tree_store_set (GTK_TREE_STORE (items_model), &iter, COLUMN_ITEM_MIQDOR,  map_prods[str][i].miqdor, -1);
                gtk_tree_store_set (GTK_TREE_STORE (items_model), &iter, COLUMN_ITEM_PROD_TICK,  map_prods[str][i].tick, -1);
            }

        }

        //cout << str << i << " " << map_prods[str][i].miqdor << endl;

    }

  return FALSE;
}

static gboolean view_row_press_cb(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
      GtkTreePath *path;
      GtkTreeViewColumn *column;
      GtkTreeIter iter, parent_iter;

      gint x, y, prodID, n=0, i, col_in;
      gint *cell_x = NULL, *cell_y = NULL;
      string str;

      //path = gtk_tree_path_new();
      //column = gtk_tree_view_column_new();
      x = event->x;
      y = event->y;

      gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(widget), x, y, &path, &column, cell_x, cell_y);

      if(column != NULL) col_in = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (column), "column"));
      //cout << col_in << endl;
      //cout << gtk_tree_view_column_get_(column) << endl;

      //gtk_tree_view_set_cursor(GTK_TREE_VIEW(widget), path, column,  FALSE);
      gtk_tree_model_get_iter(items_model, &iter, path);
      gtk_tree_model_get (items_model, &iter, COLUMN_ITEM_NUMBER, &prodID, -1);


      //cout << x << " " << y << " " << prodID << endl;
      if(prodID != 0 && col_in < 7 && !admin_mode){
      gchar *prodName=new gchar[WORD_SIZE];


        gtk_tree_model_iter_parent(items_model, &parent_iter, &iter);
        gtk_tree_model_get(items_model, &parent_iter, COLUMN_ITEM_PRODUCT, &prodName, -1);

        str.assign(prodName);
        //cout << str << endl;
        i=0;
        while(map_prods[str][i].number != prodID)
        {
            i++;
        }

        delete [] prodName;

        enter_miqdor(&n);
        //cout << n << endl;
        map_prods[str][i].miqdor = n;
        map_prods[str][i].miqdor > 0 ? map_prods[str][i].tick = true :
                                                                 map_prods[str][i].tick = false;
        gtk_tree_store_set (GTK_TREE_STORE (items_model), &iter, COLUMN_ITEM_MIQDOR,  n/*map_prods[str][i].miqdor*/, -1);
        gtk_tree_store_set (GTK_TREE_STORE (items_model), &iter, COLUMN_ITEM_PROD_TICK,  map_prods[str][i].tick, -1);

      }

      return FALSE;
}

static void row_select_cb(GtkTreeView *tree_view,
                           /*GtkTreePath  *path,
                           GtkTreeViewColumn  *col,*/
                           gpointer data )
{
    GtkTreePath  *path;
    GtkTreeIter iter, parent_iter;
    gchar *prodName;
    gint prodID;

    /*gtk_tree_view_get_cursor(tree_view, &path, NULL);

    gtk_tree_model_get_iter(items_model, &iter, path);

    gtk_tree_model_get (items_model, &iter,
                                    COLUMN_ITEM_NUMBER, &prodID, -1);*/
    //cout << "test" << prodID <<endl;
    //g_free(prodName);

    //gtk_tree_path_free(path);
}

static void tree_select_cb(GtkTreeSelection *sel, gpointer data)
{
   GtkTreeModel *prop_model;
   GtkTreePath  *path;
   GtkTreeIter  iter;
   GtkTreeViewColumn *focus_col;
   gint num;

   //prop_model = (GtkTreeModel*)data;

    //gtk_tree_selection_set_mode (sel,    GTK_SELECTION_SINGLE);
    gtk_tree_view_get_cursor(GTK_TREE_VIEW(treeview), &path, NULL);
    gtk_tree_model_get_iter (items_model, &iter, path);

    focus_col = gtk_tree_view_get_column(GTK_TREE_VIEW(treeview), 7);

   if(gtk_tree_selection_get_selected(sel, &prop_model, &iter))
    {
        //gtk_tree_model_get (items_model, &iter, COLUMN_ITEM_NUMBER, &num, -1);
        //cout << "Selected! " << num <<endl;
        //gtk_widget_grab_focus(treeview);
        gtk_tree_view_set_cursor_on_cell(GTK_TREE_VIEW(treeview), path, focus_col, NULL, FALSE);
        //g_signal_emit_by_name(GTK_WINDOW(win), "key-press-event" );
    }

    //gtk_tree_selection_set_mode (sel,    GTK_SELECTION_MULTIPLE);
}

//wrap shell of gtk message dialog
void MessageBox( string &msg, GtkMessageType msgtype)
{
    GtkWidget *msgbox;
    //cout << "TKL: "<< msg << endl;

    msgbox = gtk_message_dialog_new(GTK_WINDOW (win), GTK_DIALOG_DESTROY_WITH_PARENT,
                                                GTK_MESSAGE_INFO,
                                                GTK_BUTTONS_OK, &msg[0]);
    gtk_dialog_run(GTK_DIALOG(msgbox));
    gtk_widget_destroy(msgbox);
}

static void untickAllfunc( bool _bzero = false)
{
    gboolean next_itr;
    GtkTreeIter iter, parent_iter;

    gtk_tree_model_get_iter_first(items_model, &parent_iter);

    map<string, vector<TreeItem> >::iterator it = map_prods.begin();
    rows = 0;
    for(; it != map_prods.end(); it++)
    {
        //cout << gtk_tree_model_iter_n_children(items_model, &parent_iter) << endl;
        //gtk_tree_model_iter_nth_child(items_model, &iter, &parent_iter, c);
      gtk_tree_model_iter_children(items_model, &iter, &parent_iter);

        for(int b = 0; b < map_prods[it->first].size(); ++b)
        {
              map_prods[it->first][b].tick = false;
              gtk_tree_store_set (GTK_TREE_STORE (items_model), &iter, COLUMN_ITEM_PROD_TICK,  map_prods[it->first][b].tick, -1);
              if(_bzero)
                {
                    map_prods[it->first][b].miqdor = 0;
                    gtk_tree_store_set (GTK_TREE_STORE (items_model), &iter, COLUMN_ITEM_MIQDOR,  map_prods[it->first][b].miqdor, -1);
                }
              next_itr = gtk_tree_model_iter_next(items_model, &iter);
        }
        next_itr = gtk_tree_model_iter_next(items_model, &parent_iter);
    }
}

static void untickAll(GtkButton *btn, gpointer data)
{
    untickAllfunc();
}

gint sum = 0, factPrice_sum;

static void newCustomer(GtkButton *butt, gpointer data )
{
   sqlite3_stmt *pStmt_prodSelled;
   time_t rawtime;

   string insert_prodSelledItems = "insert into prods_selled_items values(:dateidx, $prodID, $amount)";
   string insert_prodSelled = "insert into prods_selled values(:dateidx, $sum, $factSum)";

  int ret = sqlite3_prepare_v2(pDB, &insert_prodSelledItems[0], -1, &pStmt, 0);
  if(ret != SQLITE_OK)
  {
    string msg_info = "insert SQL statement has error...";
    MessageBox(msg_info);
    return;
  }

  ret = sqlite3_prepare_v2(pDB, &insert_prodSelled[0], -1, &pStmt_prodSelled, 0);

    map<string, vector<TreeItem> >::iterator it = map_prods.begin();
    rows = 0;

    for(; it != map_prods.end(); it++)
    {
      //gtk_tree_model_iter_children(items_model, &iter, &parent_iter);

        for(int b = 0; b < map_prods[it->first].size(); ++b)
        {
             if( map_prods[it->first][b].miqdor != 0 )
                {

                    ret = sqlite3_bind_text(pStmt, 1, &gTimeStr[0], -1,  NULL);
                    ret = sqlite3_bind_int(pStmt, 2, map_prods[it->first][b].number);//prodID
                    ret = sqlite3_bind_int(pStmt, 3, map_prods[it->first][b].miqdor);//amount

                    ret = sqlite3_bind_text(pStmt_prodSelled, 1, &gTimeStr[0], -1,  NULL);
                    ret = sqlite3_bind_int(pStmt_prodSelled, 2, sum);//sum
                    ret = sqlite3_bind_int(pStmt_prodSelled, 3, factPrice_sum);//factSum

                    if( ret != SQLITE_OK) cout << "Error in bind" <<endl;

                    ret = sqlite3_step(pStmt);
                    if(ret != SQLITE_DONE)
                    {
                        string msg_info = "SQL command evaluation not done...";
                        MessageBox(msg_info);
                        return;
                    }

                    ret = sqlite3_step(pStmt_prodSelled);

                    sqlite3_reset(pStmt);
                    sqlite3_reset(pStmt_prodSelled);
                }
        }
    }

    untickAllfunc();
   //time( &rawtime );
   //pTime = localtime( &rawtime );

   //strftime(&str[0],WORD_SIZE,"%Y%m%d%H%M", pTime);
   cout << gTimeStr << endl;

}

static void calc_prods( GtkButton *butt, gpointer data)
{
    GtkTreePath *path;// = gtk_tree_path_new_from_string ("0");
    GtkTreeIter iter, parent_iter;
    GtkTextIter txt_iter;
    //GtkTextBuffer *buffer = GTK_TEXT_BUFFER(data);
    GtkTextBuffer *buffer;
    GtkWidget *view = GTK_WIDGET(data);

    gint *n, path_depth;

    //path = gtk_tree_path_new_from_string("0:0");
    //path = gtk_tree_path_new_first();
    //gtk_tree_view_get_cursor(GTK_TREE_VIEW(treeview), &path, NULL);
    //gtk_tree_path_append_index(path, 1);
    //gtk_tree_path_prepend_index(path, 1);

    //n = gtk_tree_path_get_indices(path);
    //path_depth = gtk_tree_path_get_depth(path);
    //cout << path_depth << " " << n[0] << n[1]<< endl;
    //cout << gtk_tree_path_to_string(path) << endl;

    //gtk_tree_model_get_iter_from_string(items_model, &iter, gPath_str);

    //gtk_tree_model_get_iter_from_string(items_model, &iter, "0:0");

    //gtk_tree_model_get_iter(items_model, &iter, path);
    gtk_tree_model_get_iter_first(items_model, &parent_iter);


    /*for(int i=0; i< map_prods.size(); i++ )
    {
        gtk_tree_model_get_iter (items_model, &iter, path);
        gtk_list_store_set (GTK_LIST_STORE (items_model), &iter, COLUMN_ITEM_MIQDOR,  0, -1);
        gtk_tree_path_next(path);
    }*/


    gboolean next_itr;
    gint i, b;
    gchar *d2str = new gchar[WORD_SIZE];

    memset(d2str, 0, strlen(d2str));
    sum = factPrice_sum = 0;

    /*for( next_itr = gtk_tree_model_get_iter_first(items_model, &iter);
         next_itr;
         next_itr = gtk_tree_model_iter_next(items_model, &iter))
    {
        cout << rows << endl;
        rows++;
    }*/

    map<string, vector<TreeItem> >::iterator it = map_prods.begin();
    rows = 0;
    for(; it != map_prods.end(); it++)
    {
        //cout << gtk_tree_model_iter_n_children(items_model, &parent_iter) << endl;
        //gtk_tree_model_iter_nth_child(items_model, &iter, &parent_iter, c);
      gtk_tree_model_iter_children(items_model, &iter, &parent_iter);

        for(int b = 0; b < map_prods[it->first].size(); ++b)
        {
             if( map_prods[it->first][b].miqdor != 0 ) {

                factPrice_sum += map_prods[it->first][b].factPrice * map_prods[it->first][b].miqdor;
                sum += map_prods[it->first][b].price * map_prods[it->first][b].miqdor;
                if( map_prods[it->first][b].tick){

                    map_prods[it->first][b].quant -= map_prods[it->first][b].miqdor;
                    //map_prods[prods[c].product][b].tick = false;

                    gtk_tree_store_set (GTK_TREE_STORE (items_model), &iter, COLUMN_ITEM_QUANT,  map_prods[it->first][b].quant, -1);
                    //gtk_tree_store_set (GTK_TREE_STORE (items_model), &iter, COLUMN_ITEM_PROD_TICK,  map_prods[prods[c].product][b].tick, -1);
                    //gtk_tree_store_set (GTK_TREE_STORE (items_model), &iter, COLUMN_ITEM_MIQDOR,  0, -1);

                    //cout << "Maps:" << map_prods[prods[c].product][b].quant << endl;

                    update = "update products set quant = ";

                    g_ascii_dtostr(d2str, 30, map_prods[it->first][b].quant);

                    update += d2str;
                    g_ascii_dtostr(d2str, 30, map_prods[it->first][b].number);

                    update += " where prodID = ";
                    update += d2str;

                    //res_query = mysql_query(&db, &update[0]);
                    int ret = sqlite3_prepare_v2(pDB, &update[0], update.length() , &pStmt, NULL);
                    ret == SQLITE_OK ? cout << "SQL statement OK!" <<endl : cout << "SQL statement Failed..." <<endl;

                    sqlite3_step(pStmt);

                }

                rows++;

            }

            next_itr = gtk_tree_model_iter_next(items_model, &iter);
            //cout << b << map_prods[prods[c].product][b].tick << map_prods[prods[c].product][b].product << endl;

        }

        next_itr = gtk_tree_model_iter_next(items_model, &parent_iter);
        /*gtk_tree_path_prepend_index(path, 1);

        n = gtk_tree_path_get_indices(path);
        path_depth = gtk_tree_path_get_depth(path);
        cout << c << " " << path_depth << " " << n[0] << n[1]<< endl;*/
    }


    /*for( i=0; i < prods.size(); i++)
    {
        if( prods[i].tick) {

            sum += prods[i].price * prods[i].miqdor;
            prods[i].quant -= prods[i].miqdor;

            //gtk_tree_store_set (GTK_TREE_STORE (items_model), &iter, COLUMN_ITEM_QUANT,  prods[i].quant, -1);
            //next_itr = gtk_tree_model_iter_next(items_model, &iter);


            /*while( row = mysql_fetch_row(mRes) ){

                row[3] = g_ascii_dtostr(d2str, 30, prods[i].quant);
            }/

            //cout << prods[i].product << update <<endl;
        }
    }*/

    //gtk_tree_model_row_changed(items_model, path, &iter);
    char *spaces;
    GdkRectangle vis_rect;
    //PangoFontDescription *font_desc;

    /*font_desc = pango_font_description_from_string("Sans 30");
    gtk_widget_modify_font(view, font_desc);*/

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
    gtk_text_buffer_set_text (buffer, "", 0);
    gtk_text_buffer_get_iter_at_offset (buffer, &txt_iter, 0);

    g_ascii_dtostr(d2str, 30, sum);

    g_strlcat(d2str, "\n", -1);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &txt_iter, d2str, -1, "big", "red_foreground" ,"wide_margins", NULL);

    //gtk_text_buffer_insert (buffer, &txt_iter, d2str, -1);

    //gtk_text_view_get_visible_rect(GTK_TEXT_VIEW(view), &vis_rect);
    //cout << vis_rect.width << endl;
    /*spaces = g_strnfill (50, ' ');
    gtk_text_buffer_insert (buffer, &txt_iter, spaces, -1);*/


    g_ascii_dtostr(d2str, 30, factPrice_sum);

    //pango_font_description_set_size(font_desc, 10);
    //gtk_widget_modify_font(view, font_desc);
    //gtk_text_buffer_insert (buffer, &txt_iter, d2str, -1);
    gtk_text_buffer_insert_with_tags_by_name (buffer, &txt_iter, d2str, -1, "right_justify", NULL);

 time_t rawtime;
   //struct tm *info;
   //char buf[80];

   time( &rawtime );

   pTime = localtime( &rawtime );

   strftime(d2str,WORD_SIZE,"%Y%m%d%H%M", pTime);
   gTimeStr.assign(d2str);

   cout << gTimeStr << endl;


    delete [] d2str;
    g_free(spaces);
    //pango_font_description_free(font_desc);
    //gtk_tree_path_free(path);
    cout << "Sum: " << sum << endl;
}

static void ProdCellingGrid(GtkWidget *wid, GtkWidget *win)
{
  GtkWidget *dialog = NULL;
}


static void db_row_newprod(GtkButton *btn, gpointer data)
{
  GtkTreeView *treeview = (GtkTreeView*)data;
  //GtkTreeModel *model = gtk_tree_view_get_model (treeview);;
  GtkTreeIter iter, parent_iter;
  GtkTreePath *path;
  gint prodID = 1;

  if(prods.size() > 0) prodID = prods[prods.size()-1].number+1;
  //gchar *prodName = new gchar[WORD_SIZE];

  string insert_sql = "insert into products values($prodID, :prodName, ' ', 0, 0, 0, ' ')";

  int ret = sqlite3_prepare_v2(pDB, &insert_sql[0], -1, &pStmt, 0);
  if(ret != SQLITE_OK)
  {
    string msg_info = "insert SQL statement has error...";
    MessageBox(msg_info);
    return;
  }

  gchar *d2str = new gchar[WORD_SIZE];

  g_ascii_dtostr(d2str, 30, prodID);

  string str = "new_prod";
  str.append(d2str);

  TreeItem foo;

  foo.number  = prodID;
  //foo.product = g_strdup( (const gchar*)str.c_str() );
  foo.product = g_convert((const gchar*)(&str[0]), -1, "UTF-8", "UTF-8", NULL, NULL, NULL);
  foo.prop    = NULL;
  foo.price   = 0;
  foo.quant   = 0;
  foo.factPrice = 0;
  foo.tick = false;
  foo.miqdor = 0;

  prods.push_back(foo);


  map_prods[&str[0]].push_back(foo);

  delete [] d2str;

  gtk_tree_model_get_iter_first(items_model, &parent_iter);

  gtk_tree_store_append (GTK_TREE_STORE(items_model), &parent_iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE(items_model), &parent_iter, COLUMN_ITEM_PRODUCT, &str[0], -1);

  gtk_tree_store_append (GTK_TREE_STORE(items_model), &iter, &parent_iter);
  gtk_tree_store_set (GTK_TREE_STORE(items_model), &iter,
                      COLUMN_ITEM_NUMBER, prodID,
                      COLUMN_ITEM_PROP, NULL,
                      COLUMN_ITEM_QUANT, 0,
                      COLUMN_ITEM_PRICE, 0.0,
                      COLUMN_ITEM_FACT_PRICE, 0.0,
                      COLUMN_ITEM_PROD_TICK, FALSE,
                      COLUMN_ITEM_MIQDOR, NULL,
                      VISIBLE_COLUMN, TRUE,
                      -1);

  sqlite3_bind_int(pStmt, 1, prodID);
  sqlite3_bind_text(pStmt, 2, foo.product, -1,  NULL);

  ret = sqlite3_step(pStmt);
  if(ret != SQLITE_DONE)
  {
    string msg_info = "SQL command evaluation not done...";
    MessageBox(msg_info);
    return;
  }

}

static void db_row_insert(GtkButton *btn, gpointer data)
{
  GtkTreeView *treeview = (GtkTreeView*)data;
  GtkTreeModel *model = gtk_tree_view_get_model (treeview);
  GtkTreeIter iter, parent_iter;
  GtkTreePath *path=NULL;
  gint prodID;
  gchar *prodName = new gchar[WORD_SIZE];

  //bsave_db = true;

  gtk_tree_view_get_cursor(GTK_TREE_VIEW(treeview), &path, NULL);

  if(path != NULL){
    gtk_tree_model_get_iter (items_model, &parent_iter, path);
    //gtk_tree_model_iter_children(items_model, &iter, &parent_iter);

    /*gtk_tree_model_get (items_model, &parent_iter,COLUMN_ITEM_PRODUCT,
                                     &prodName, COLUMN_ITEM_NUMBER, &prodID, -1);*/
    //cout << prodName<<" " << prodID <<endl;
    //gtk_tree_model_get (items_model, &parent_iter, COLUMN_ITEM_PRODUCT, &prodName, -1);

    //if(prodID == 0 )
    if(gtk_tree_model_iter_has_child(items_model, &parent_iter))
    {
        cout << "3 " ;
        //gtk_tree_model_get_iter (items_model, &parent_iter, path);
        gtk_tree_model_iter_children(items_model, &iter, &parent_iter);

        gtk_tree_model_get (items_model, &parent_iter,COLUMN_ITEM_PRODUCT, &prodName, -1);
        gtk_tree_model_get (items_model, &iter, COLUMN_ITEM_NUMBER, &prodID, -1);

        //g_print(prodName);
        //g_free(prodName);
        //prodName = NULL;
        cout << prodID << " " << prodName<<endl;

    }
    else{
            cout <<"2 ";
            gtk_tree_model_iter_parent(items_model, &iter, &parent_iter);
            gtk_tree_model_get (items_model, &iter, COLUMN_ITEM_PRODUCT, &prodName, -1);
            //gtk_tree_model_get (items_model, &parent_iter, COLUMN_ITEM_NUMBER, &prodID, -1);

            cout << prodName << " " << prodID << endl;
    }
    //if(prodName != NULL)  {      g_print(prodName);    }
    //cout << prodID  << " " << prods[prods.size()-1].number << endl;
  }
  else
    {
        string msg_info = "Please choose the product or press the New Prod button.";
        MessageBox(msg_info);
        return;
    }

    prodID = prods[prods.size()-1].number+1;

  string insert_sql = "insert into products values($prodID, :prodName, ' ', 0, 0, 0, ' ')";

  int ret = sqlite3_prepare_v2(pDB, &insert_sql[0], -1, &pStmt, 0);
  if(ret != SQLITE_OK)
  {
    string msg_info = "insert SQL statement has error...";
    MessageBox(msg_info);
    return;
  }

  TreeItem foo;

  //prodID = prods.size(); ++prodID;

  foo.number  = prodID;
  foo.product = g_strdup(prodName);
  //foo.product = g_convert((const gchar*)(&prodName[0]), -1, "UTF-8", "UTF-8", NULL, NULL, NULL);
  foo.prop    = NULL;
  foo.price   = 0;
  foo.quant   = 0;
  foo.factPrice = 0;
  foo.tick = false;
  foo.miqdor = 0;

  sqlite3_bind_int(pStmt, 1, prodID);
  sqlite3_bind_text(pStmt, 2, prodName, -1,  SQLITE_STATIC);

  //prods.assign(prodID, foo);
  prods.push_back(foo);
  map_prods[foo.product].push_back(foo);

  ret = sqlite3_step(pStmt);
  if(ret != SQLITE_DONE)
  {
    string msg_info = "SQL command evaluation not done...";
    MessageBox(msg_info);
    return;
  }
  //sqlite3_reset(pStmt);
  //sqlite3_clear_bindings(pStmt);


  //gtk_tree_store_append (GTK_TREE_STORE(model), &parent_iter, NULL);
  //gtk_tree_store_insert(GTK_TREE_STORE(model), &parent_iter, NULL,1);
  //gtk_tree_store_set (GTK_TREE_STORE(model), &parent_iter, COLUMN_ITEM_PRODUCT, prodName, -1);

    if(gtk_tree_store_is_ancestor(GTK_TREE_STORE(model), &parent_iter, &iter) )
        {
                gtk_tree_store_append (GTK_TREE_STORE(model), &iter, &parent_iter);
                cout << "Yes " << gtk_tree_model_get_string_from_iter(model, &iter) << endl;
        }
    else
        {
            //gtk_tree_store_insert(GTK_TREE_STORE(model), &parent_iter, NULL, -1);
            gtk_tree_model_iter_parent(items_model, &iter, &parent_iter);
            gtk_tree_store_append (GTK_TREE_STORE(model), &parent_iter, &iter);
            iter = parent_iter;
            cout << "NO " << gtk_tree_model_get_string_from_iter(model, &iter)<< endl;
        }

  gtk_tree_store_set (GTK_TREE_STORE(model), &iter,
                      COLUMN_ITEM_NUMBER, prodID,
                      COLUMN_ITEM_FACT_ID, NULL,
                      COLUMN_ITEM_PROP, NULL,
                      COLUMN_ITEM_QUANT, 0,
                      COLUMN_ITEM_PRICE, 0.0,
                      COLUMN_ITEM_FACT_PRICE, 0.0,
                      COLUMN_ITEM_PROD_TICK, FALSE,
                      COLUMN_ITEM_MIQDOR, NULL,
                      VISIBLE_COLUMN, TRUE,
                      -1);


  //gtk_tree_model_get_iter_first(items_model, &parent_iter);

  //g_free(foo.product);
    delete [] prodName;
    gtk_tree_path_free(path);
}

static void db_row_del(GtkButton *btn, gpointer data)
{

  GtkTreeView *treeview = (GtkTreeView*)data;
  GtkTreeModel *model = gtk_tree_view_get_model (treeview);;
  GtkTreeIter iter, parent_iter;
  GtkTreePath *path;
  gint prodID, pos=0, i, count_prods;
  gchar *prodName = new gchar[WORD_SIZE];
  gboolean all_or_one=FALSE;

  //bsave_db = true;

  gtk_tree_view_get_cursor(GTK_TREE_VIEW(treeview), &path, NULL);

  if(path != NULL){

    gtk_tree_model_get_iter (model, &parent_iter, path);

    gtk_tree_model_get (model, &parent_iter,COLUMN_ITEM_PRODUCT,
                                     prodName, COLUMN_ITEM_NUMBER, &prodID, -1);
    //cout << prodID << endl;

    if(prodID == 0 )
    {
        all_or_one = TRUE;

        bool res = gtk_tree_model_iter_children(model, &iter, &parent_iter);

        //if( res )
            {
            gtk_tree_model_get (model, &parent_iter,COLUMN_ITEM_PRODUCT, &prodName, -1);
            gtk_tree_model_get (model, &iter, COLUMN_ITEM_NUMBER, &prodID, -1);
            cout << "2 " << prodName << prodID << endl;
            }


    }
    else{   //single row
            all_or_one = FALSE;
            gtk_tree_model_iter_parent(items_model, &iter, &parent_iter);
            gtk_tree_model_get (items_model, &iter, COLUMN_ITEM_PRODUCT, &prodName, -1);
            cout << "3 " << prodName << prodID <<endl;
    }

    //if( strlen(prodName) > 0 )// && prodID == 0)
    {

        //map_prods.erase(prodName);
        //g_print(prodName);


        /*
        pos=0;
        while(prods[pos].number != prodID)
            {
                ++pos;
            }
        prods.erase(prods.begin()+pos);*/
        //g_free(prodName);
        //delete [] prodName;
    }
    //cout << prodName <<"\n Size: "<< prods.size() << endl;

    //--prodID;
    //prods.erase(prods.begin()+prodID);
    //++prodID;

  }
  else{
    string msg_info = "Please choose the product or press the New Prod button.";
    MessageBox(msg_info);
    return;
  }

  string del_sql;
  string str;
  str.assign(prodName);

  if(all_or_one)
  {

        map_prods.erase(map_prods.find(str));

        del_sql = "delete from products where prod_name = :prodName";

    int ret = sqlite3_prepare_v2(pDB, &del_sql[0], -1, &pStmt, 0);
    if(ret != SQLITE_OK)
    {
        string msg_info = "delete SQL statement has error...";
        MessageBox(msg_info);
        return;
    }

    sqlite3_bind_text(pStmt, 1, prodName, -1,  SQLITE_STATIC);

  //map_prods[prods[prodID].product].erase(prods.begin()+prodID);

    i=pos=0;
    count_prods=prods.size();
    while( i < count_prods+1 )
    {
        if(g_strcmp0(prodName, prods[i].product)==0 ){
        cout << i << prods[i].product << count_prods <<endl;
                prods.erase(prods.begin()+i);
                count_prods=prods.size();
        }
        //++pos;

        //sqlite3_bind_int(pStmt, 1, prodID);


        //cout << "T:"<<prods[prodID].product << endl;

        //for(int i = 0; i < prods.size(); i++)
        //cout << prods[i].product << endl;


        ret = sqlite3_step(pStmt);
        if(ret != SQLITE_DONE)
        {
            string msg_info = "SQL command evaluation not done...";
            MessageBox(msg_info);
            return;
        }
        sqlite3_reset(pStmt);
        i++;
    }

  }
  else
  {
        del_sql = "delete from products where prodID = $prodID";

        int ret = sqlite3_prepare_v2(pDB, &del_sql[0], -1, &pStmt, 0);
        if(ret != SQLITE_OK)
        {
            string msg_info = "delete SQL statement has error...";
            MessageBox(msg_info);
            return;
        }

        sqlite3_bind_int(pStmt, 1, prodID);

        ret = sqlite3_step(pStmt);
        if(ret != SQLITE_DONE)
        {
            string msg_info = "SQL command evaluation not done...";
            MessageBox(msg_info);
            return;
        }
        i=0;
        while(map_prods[str][i].number != prodID)
        {
            i++;
        }
        map_prods[str].erase(map_prods[str].begin()+i);

        for(i = 0; i < prods.size(); i++)
        {
            if(prods[i].number == prodID)
            {
                prods.erase(prods.begin()+i);
                break;
            }
        }

    //gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
  }

  //cout << "Size: " << prods.size() << endl;

    gtk_tree_path_free(path);
    g_free(prodName);
    gtk_tree_store_remove(GTK_TREE_STORE(model), &parent_iter);

}

void db_prods_saled_export_xls(GtkButton *btn, gpointer data)
{

 GtkWidget *dialog, *cal_dlg;
 GtkWidget *content_area;
  GtkWidget *hbox;
  GtkWidget *stock;
  GtkWidget *table;
  GtkWidget *cal = NULL;
 SqliteExport *se = 0;


cal_dlg = gtk_dialog_new_with_buttons ("Interactive Dialog",
                                        GTK_WINDOW (win),
                                        static_cast<GtkDialogFlags>( GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
                                        GTK_STOCK_OK,
                                        GTK_RESPONSE_OK,
                                        GTK_STOCK_CANCEL,
                                        GTK_RESPONSE_CANCEL,
                                        NULL);

content_area = gtk_dialog_get_content_area (GTK_DIALOG (cal_dlg));

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 8);
  gtk_box_pack_start (GTK_BOX (content_area), hbox, FALSE, FALSE, 0);

  stock = gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start (GTK_BOX (hbox), stock, FALSE, FALSE, 0);

table = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (table), 4);
  gtk_grid_set_column_spacing (GTK_GRID (table), 4);
  gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, 0);

  guint y, m, d;
  cal = gtk_calendar_new ();
  //gtk_calendar_mark_day(GTK_CALENDAR(cal), 11);
  //gtk_calendar_select_day(GTK_CALENDAR(cal), 11);

  gtk_box_pack_start (GTK_BOX (hbox), cal, TRUE, TRUE, 0);

  gtk_widget_show_all (hbox);
gint response = gtk_dialog_run (GTK_DIALOG (cal_dlg));

string d2str, sdate;
d2str.resize(5);
  if (response == GTK_RESPONSE_OK)
    {
      //gtk_entry_set_text (GTK_ENTRY (entry1), gtk_entry_get_text (GTK_ENTRY (local_entry1)));
      //gtk_entry_set_text (GTK_ENTRY (entry2), gtk_entry_get_text (GTK_ENTRY (local_entry2)));
      gtk_calendar_get_date(GTK_CALENDAR(cal),&y, &m, &d);

      cout << y << ++m << d << endl;
      g_ascii_dtostr(&d2str[1], d2str.size(), (gint)y);
      //sdate = d2str;
      sdate.assign(&d2str[1]);

      if(m < 10) sdate.append("0");
      g_ascii_dtostr(&d2str[1], d2str.size(), (gint)m);
      sdate.append(&d2str[1]);

      if(d < 10) sdate.append("0");
      g_ascii_dtostr(&d2str[1], d2str.size(), (gint)d);
      sdate.append(&d2str[1]);
      sdate.insert(0,1,'\'');
      sdate.append("%'");
      cout << "Str:" << sdate << endl;

      dialog = gtk_file_chooser_dialog_new ("Save File",
                                      GTK_WINDOW(win),
                                      GTK_FILE_CHOOSER_ACTION_SAVE,
                                      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                      NULL);
        gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

        if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
        {
            se = new SqliteExportHtml();
            bool success = false;
            char *filename;

            filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
            if(se != 0)
            {
                //char query[1024] = "select prods_selled_items.dateidx, prods_selled_items.prodID, products.prod_feature, prods_selled_items.amount, prods_selled.sum from products, prods_selled, prods_selled_items where prods_selled_items.prodID=products.prodID and prods_selled_items.dateidx=prods_selled.dateidx and prods_selled_items.dateidx like '201412%'";//
                string query = "select prods_selled_items.dateidx, prods_selled_items.prodID, products.prod_feature, prods_selled_items.amount, prods_selled.sum from products, prods_selled, prods_selled_items where prods_selled_items.prodID=products.prodID and prods_selled_items.dateidx=prods_selled.dateidx and prods_selled_items.dateidx like ";
                //char query[1024] = "select * from prods_selled";
                string database = "Prods_Prices";
                query.append(&sdate[0]);
                cout << query << endl;
                success = se->DoExport(&database[0], filename, &query[0]);
                delete se;
                se = 0;
            }
            //save_to_file (filename);
            //ofstream outf(filename, ios::out | ios::trunc);
            //outf << "test" << endl;
            //outf.close();
            g_free (filename);
        }

        gtk_widget_destroy (dialog);

    }

  gtk_widget_destroy (cal_dlg);


/*if (user_edited_a_new_document)
  gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "Untitled document");
else
  gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), filename_for_existing_document);*/

}

void gtk_my_quit()
{
  //sqlite3_clear_bindings(pStmt);

    if(bsave_db)
    {
        cout << "updating...." << endl;
        string sql_cmd = "update products set prod_name=:prodName, prod_feature = :prop, price = $price, factPrice = $factPrice, quant = $quant, factProdID = :factProdID where prodID = $prodID";
        //string sql_cmd = "update products set prod_feature = :prop, price = $price, factPrice = $factPrice, quant = $quant where prodID = $prodID";

        int ret = sqlite3_prepare_v2(pDB, &sql_cmd[0], -1, &pStmt, 0);
        if(ret != SQLITE_OK)
        {
            string msg_info = "update SQL statement has error...";
            MessageBox(msg_info);
            return;
        }

  //gchar *str = new gchar[WORD_SIZE];
  map<string, vector<TreeItem> >::iterator it = map_prods.begin();

    //for(int c=0; c < map_prods.size(); c++ )
    for(; it != map_prods.end(); it++)
    {
        //cout << it->first << endl;
        //for(int b = 0; b < map_prods[prods[c].product].size(); b++)
        for(int b = 0; b < map_prods[it->first].size(); ++b)
            {

            //if( map_prods[prods[c].product][b].tick) {     }
            //g_ascii_dtostr(str, 30, b);
                //cout << prods[c].product << " " << map_prods[prods[c].product][b].prop  << endl;

                ret = sqlite3_bind_text(pStmt, 1, &(it->first[0]), -1,  NULL);
                ret = sqlite3_bind_text(pStmt, 2, map_prods[it->first][b].prop, -1,  NULL);
                ret = sqlite3_bind_text(pStmt, 6, map_prods[it->first][b].factProdID, -1,  NULL);
                //ret = sqlite3_bind_text(pStmt, 1, prods[b].prop, 3,  NULL);
                if( ret != SQLITE_OK) cout << "Error in bind" <<endl;

                sqlite3_bind_double (pStmt, 3, map_prods[it->first][b].price);
                sqlite3_bind_double (pStmt, 4, map_prods[it->first][b].factPrice);
                sqlite3_bind_int (pStmt, 5, map_prods[it->first][b].quant);
                sqlite3_bind_int (pStmt, 7, map_prods[it->first][b].number);

                ret = sqlite3_step(pStmt);
                if(ret != SQLITE_DONE)
                {
                    string msg_info = "SQL command evaluation not done...";
                    MessageBox(msg_info);
                    return;
                }
                sqlite3_reset(pStmt);
            }
        }
    }
  //delete [] str;
  gtk_main_quit();
}
//static GdkColor text_col_color;

int main (int argc, char *argv[])
{
  PangoFontDescription *font_desc;
  GtkWidget *view;
  GtkTextBuffer *buffer;

  GtkWidget *button = NULL, *check_button;
//  GtkWidget *win = NULL;
  GtkWidget *vbox = NULL, *frame_vert;
  GtkWidget *hbox = NULL;
  GtkWidget *sw   = NULL;
  //GtkWidget *treeview = NULL;

  //gPath_str = new gchar[32];

  /* Initialize GTK+ */
  g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc) gtk_false, NULL);
  gtk_init (&argc, &argv);
  g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);

  /* Create the main window */
  win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width (GTK_CONTAINER (win), 3);
  gtk_window_set_title (GTK_WINDOW (win), "GDo'kon");
  gtk_window_set_position (GTK_WINDOW (win), GTK_WIN_POS_CENTER);

  //gtk_widget_realize (win);
  g_signal_connect (win, "destroy", gtk_main_quit, NULL);

  int ret = InitDB();
  if(ret < 0 ) return 0;

  //gtk_window_resize(GTK_WINDOW(win), 600, 360);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
  gtk_container_add (GTK_CONTAINER (win), vbox);


  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  //gtk_container_add (GTK_CONTAINER (win), hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 6);

  /*gtk_box_pack_start (GTK_BOX (hbox),
                         gtk_label_new ("Shopping list !"),
                         FALSE, FALSE, 0);*/

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
                                           GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                      GTK_POLICY_AUTOMATIC,
                                      GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (hbox), sw, TRUE, TRUE, 5);


  /* create models */
  items_model   = create_items_model ();
  //numbers_model = create_numbers_model();

  /* create tree view */
  treeview = gtk_tree_view_new_with_model (items_model);


  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);

  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)),
                                   GTK_SELECTION_SINGLE);

  //g_signal_connect(G_OBJECT(treeview), "cursor-changed", G_CALLBACK(row_select_cb), NULL);
  g_signal_connect(treeview, "key-press-event", G_CALLBACK(key_press_cb), NULL);
  g_signal_connect(treeview, "button-press-event", G_CALLBACK(view_row_press_cb), NULL);

  add_columns (GTK_TREE_VIEW (treeview)/*, items_model, numbers_model*/);
  g_object_unref (items_model);

  /*gtk_tree_selection_selected_foreach(gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)),
                                       selected_foreach, NULL);*/

  tree_select = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
  //g_signal_connect(G_OBJECT(tree_select), "changed", G_CALLBACK(tree_select_cb), items_model);

  //g_object_unref (numbers_model);

  gtk_container_add (GTK_CONTAINER (sw), treeview);

  /* expand all rows after the treeview widget has been realized */
  g_signal_connect (treeview, "realize",
                        G_CALLBACK (gtk_tree_view_expand_all), NULL);

  //gtk_window_set_default_size (GTK_WINDOW (win), 750, 450);
  gtk_window_maximize ( GTK_WINDOW(win) );

  /*Total prices of products */
  frame_vert = gtk_frame_new ("Total:");
  gtk_widget_set_valign (frame_vert, GTK_ALIGN_START);
  gtk_widget_set_size_request(frame_vert, 0, 100);
  gtk_box_pack_start(GTK_BOX (vbox), frame_vert, FALSE, FALSE, 8);

  view = gtk_text_view_new ();
  gtk_text_view_set_editable(GTK_TEXT_VIEW (view), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);

  //font_desc = pango_font_description_from_string("Sans 30");
  //font_desc = pango_font_description_new();
  //pango_font_description_set_family(font_desc, "Sans");
  //pango_font_description_set_size(font_desc, 30);
  //gtk_widget_modify_font(view, font_desc);
  //pango_font_description_free(font_desc);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
  gtk_text_buffer_create_tag (buffer, "left", "justification", GTK_JUSTIFY_LEFT, NULL);
    gtk_text_buffer_create_tag (buffer, "right_justify", "justification", GTK_JUSTIFY_RIGHT, NULL);
    gtk_text_buffer_create_tag (buffer, "big_gap_before_line", "pixels_above_lines", 30, NULL);
    gtk_text_buffer_create_tag (buffer, "big_gap_after_line", "pixels_below_lines", 30, NULL);
    gtk_text_buffer_create_tag (buffer, "wide_margins",  "left_margin", 50, "right_margin", 50, NULL);
    gtk_text_buffer_create_tag (buffer, "red_foreground", "foreground", "red", NULL);
    gtk_text_buffer_create_tag (buffer, "big",
                              /* points times the PANGO_SCALE factor */
                              "size", 70 * PANGO_SCALE, NULL);

  gtk_container_add (GTK_CONTAINER (frame_vert), view);

  /* Create a vertical box with buttons */
  frame_vert = gtk_frame_new ("Controls:");
  gtk_widget_set_valign (frame_vert, GTK_ALIGN_START);
  gtk_box_pack_end(GTK_BOX (hbox), frame_vert, FALSE, FALSE, 10);

  //vbox = gtk_vbox_new (TRUE, 7);
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
  gtk_container_add (GTK_CONTAINER (frame_vert), vbox);

  /* some buttons */
  //hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  //gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
  //gtk_container_add (GTK_CONTAINER (frame_vert), hbox);
  //gtk_box_set_homogeneous (GTK_BOX (hbox), TRUE);
  //gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  /* And a check button to turn grouping on and off */
    check_button = gtk_check_button_new_with_mnemonic ("Ad_ministrating");
    gtk_box_pack_start (GTK_BOX (vbox), check_button, FALSE, FALSE, 0);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button), FALSE);
    g_signal_connect (check_button, "toggled",
                        G_CALLBACK (toggle_admin), NULL);


  db_new_but = gtk_button_new_with_mnemonic("_New product");
  gtk_widget_set_sensitive(db_new_but, FALSE);
  g_signal_connect (db_new_but, "clicked", G_CALLBACK(db_row_newprod), treeview);
  gtk_box_pack_start (GTK_BOX (vbox), db_new_but, FALSE, FALSE, 2);

  db_ins_but = gtk_button_new_with_mnemonic("_Insert");
  gtk_widget_set_sensitive(db_ins_but, FALSE);
  g_signal_connect (db_ins_but, "clicked", G_CALLBACK(db_row_insert), treeview);
  gtk_box_pack_start (GTK_BOX (vbox), db_ins_but, FALSE, FALSE, 2);

  db_del_but = gtk_button_new_with_mnemonic("De_lete");
  gtk_widget_set_sensitive(db_del_but, FALSE);
  g_signal_connect (db_del_but, "clicked", G_CALLBACK(db_row_del), treeview);
  gtk_box_pack_start (GTK_BOX (vbox), db_del_but, FALSE, FALSE, 2);

  db_export_but = gtk_button_new_with_mnemonic("E_xport2Xls");
  gtk_widget_set_sensitive(db_export_but, FALSE);
  g_signal_connect (db_export_but, "clicked", G_CALLBACK(db_prods_saled_export_xls), GTK_WINDOW(win));
  gtk_box_pack_start (GTK_BOX (vbox), db_export_but, FALSE, FALSE, 2);

  button = gtk_button_new_from_stock (GTK_STOCK_APPLY);
  //g_signal_connect (button, "clicked", G_CALLBACK(calc_prods), buffer);
  g_signal_connect (button, "clicked", G_CALLBACK(calc_prods), view);
  //gtk_container_add (GTK_CONTAINER (frame_vert), button);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 3);
  //gtk_widget_show(button);

  button = gtk_button_new_with_mnemonic("Untic_k");
  g_signal_connect (button, "clicked", G_CALLBACK(untickAll), GTK_WINDOW(win));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 3);

  button = gtk_button_new_from_stock(GTK_STOCK_NEW);
  g_signal_connect (button, "clicked", G_CALLBACK(newCustomer), GTK_WINDOW(win));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 3);

  button = gtk_button_new_from_stock (GTK_STOCK_PRINT);
  g_signal_connect (button, "clicked", G_CALLBACK(do_printing), GTK_WINDOW(win));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 3);

  //gtk_size_group_add_widget (group, button);
  //gtk_container_add (GTK_CONTAINER (button), gtk_image_new_from_stock (GTK_STOCK_ADD, (GtkIconSize)1));

  //g_signal_connect (button, "clicked", G_CALLBACK(calc_prods), buffer);

  button = gtk_button_new_with_mnemonic("Abou_t");
  g_signal_connect (button, "clicked",	G_CALLBACK (about_us), GTK_WINDOW(win));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 1);

  button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  g_signal_connect (button, "clicked", gtk_my_quit, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 1);

  /* Enter the main loop */
  gtk_widget_show_all (win);
  gtk_main ();


  //delete [] gPath_str;
  //mysql_close(&db);


  ret = sqlite3_finalize(pStmt);
  ret == SQLITE_OK ? cout << "statement finalized" <<endl : cout << "Failed..." <<endl;

  ret = sqlite3_close_v2(pDB);
  ret == SQLITE_OK ? cout << "DB closed" <<endl : cout << "Closing Failed..." <<endl;

  //mysql_server_end();


  return 0;
}

int InitDB()
{
  int ret;

  ret = sqlite3_open_v2("Prods_Prices", &pDB, SQLITE_OPEN_READWRITE, NULL);
  ret == SQLITE_OK ? cout << "DB connected OK!" <<endl : cout << "Failure..." <<endl;

  //ret = sqlite3_open_v2("prods_bought", &pProdsBoughtDB, SQLITE_OPEN_READWRITE, NULL);
  //ret == SQLITE_OK ? cout << "ProdsBoughtDB connected OK!" <<endl : cout << "Failure..." <<endl;


  string errmsg;
  //my_ulonglong i=0;

  //mysql_init(&db);

  //MYSQL *db_conn = mysql_real_connect(&db, "localhost", "azizbek", "azizka", NULL, 0, mysql_unix_port, 0);

  /*if(db_conn == NULL) {
    errmsg.assign(mysql_error(&db));
    MessageBox(errmsg);
    return -1;
  }*/

  /*if(mysql_select_db(&db, "Pipes_Prods") > 0)
  {
    //printf("Error %u: %s\n", mysql_errno(&db), mysql_error(&db));
    //errmsg = "Pipes_Prods table not found";
    errmsg.assign(mysql_error(&db));
    MessageBox(errmsg);

    /*if(mysql_query(&db,"CREATE DATABASE Pipes_Prods"))
    {
      printf("Error %u: %s\n", mysql_errno(&db), mysql_error(&db));
      return -1;
    }--/
    return -1;
  }*/

  /*res_query = mysql_query(&db, &query[0]);

  //mRes = mysql_use_result(&db);
  mRes = mysql_store_result(&db);
  mFld = mysql_fetch_fields(mRes);

  /*char **row = mysql_fetch_row(mRes);

  for(i=0; i < mysql_num_fields(mRes); i++)
      cout << mFld[i].name << " ";
  endl(cout);

   for(i=0; i < mysql_num_fields(mRes); i++)
      cout << row[i]<< " ";

  endl(cout);

  int b = atol(row[3]);
  cout << --b << endl;*/
}

static void
toggle_admin (GtkToggleButton *check_button,
                 GtkSizeGroup    *size_group)
{
  if (gtk_toggle_button_get_active (check_button))
    admin_mode = true;
  else
    admin_mode = false;
  gtk_widget_set_sensitive(db_del_but, admin_mode);
  gtk_widget_set_sensitive(db_ins_but, admin_mode);
  gtk_widget_set_sensitive(db_new_but, admin_mode);
  gtk_widget_set_sensitive(db_export_but, admin_mode);

  for(int i=0; i < 6; i++)
  {
    g_object_set (arenderer[i], "editable", admin_mode,  NULL);
   }
}

static void
item_toggled (GtkCellRendererToggle *cell,
              gchar                 *path_str,
              gpointer               data)
{
  GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  GtkTreeIter iter, parent_iter;
  gboolean toggle_item;

  gint *column, prod_index;

  //column = reinterpret_cast<gint*>(g_object_get_data (G_OBJECT (cell), "column"));
  column = (gint*)g_object_get_data (G_OBJECT (cell), "column");

  /* get toggled iter */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_iter_parent(items_model, &parent_iter, &iter);

  gtk_tree_model_get (model, &iter, column, &toggle_item, -1);

  gchar *prodName = new gchar[WORD_SIZE];

  gtk_tree_model_get(items_model, &parent_iter, COLUMN_ITEM_PRODUCT, &prodName, -1);

  gtk_tree_model_get(items_model, &iter, COLUMN_ITEM_NUMBER, &prod_index, -1);

  //cout << prodName << prod_index << endl;

  /* do something with the value */
  toggle_item ^= 1;

  /* set new value */
  //gtk_list_store_set(GTK_LIST_STORE( (GtkListStore*)data ), &iter, column, toggle_item, -1);
  //prods[prod_index-1].tick = toggle_item;

  int i=0;
  string str;
  str.assign(prodName);
  while(map_prods[str][i].number != prod_index)
  {
    i++;
  }
  map_prods[str][i].tick = toggle_item;
  //cout << map_prods[prods[prod_index-1].product][i].number << i <<endl;

  gtk_tree_store_set (GTK_TREE_STORE (model), &iter, column, toggle_item, -1);

  /* clean up */
  delete [] prodName;
  gtk_tree_path_free (path);
}


vector<int> indx;

static void fill_prodcmb_entry (GtkWidget *combo)
{
  char **row, *props[10];
  int c=0;
  vector<string> tmp(10);
  string str;

  //map<string, vector<string> >::iterator it = map_prods.begin();
  //vector<string>::iterator st = it->second.begin();

  /*for(; it != map_prods.end(); ++it)
  {
    //cout << (*it).first << " ";//<< it->second.at(c) << endl;
    for(c=0; c < it->second.size(); c++)
     cout << (*it).first << it->second.at(c) <<endl;
    /*for(; st != it->second.end(); st++)
        cout << *st << endl;*/
  //}

      //while( c < articles->len )
      {

        /*if( strcmp(g_array_index (articles, Item, c).product, g_array_index (articles, Item, 1+c).product))
        {
          //props[c] = g_strdup(row[2]);

          //cout << g_array_index (articles, Item, c).product << endl;
        }*/
        //printf("%d", indx[c]);
        //if(g_strcmp0(prods[c].product) )
        //c++;
      }

}

static void selected_foreach( GtkTreeModel *items_model,
                              GtkTreePath  *path,
                              GtkTreeIter  *iter,
                              gpointer     data )
{

    gchar *name;
    //gtk_tree_model_get (items_model, iter, COLUMN_ITEM_PRODUCT, &name, -1);

    string str(name);
    //vector<string> vprods = map_prods[str];
    printf("TEST%s", name);

    //for(int i=0; i < vprods.size(); i++)
    {
        //cout << vprods[i] << endl;
    }


   //g_free(name);
}

static GtkTreeModel * create_numbers_model (void)
{
#define N_NUMBERS 10
  //map<string, vector<string> >::iterator it = map_prods.begin();

  int i = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  //vector<string> vprops;

  /* create list store */
  //model = gtk_list_store_new (NUM_NUMBER_COLUMNS, G_TYPE_STRING);

  /* add numbers */

/*  for(; it != map_prods.end(); ++it)
  {
    //cout << (*it).first << " ";//<< it->second.at(c) << endl;
    vprops = map_prods[(*it).first];

    for(i=0; i < vprops.size(); i++)
    {
      gtk_list_store_append (model, &iter);
      gtk_list_store_set (model, &iter,
                          COLUMN_PROP_COMBO, reinterpret_cast<gchar*>(&vprops[i][0])/*prods[i].prop*,
                          -1);
      //cout << vprops[i] << endl;
      //cout << (*it).first << vprops[i] <<endl;
    }
     vprops.clear();
  }*/

  return GTK_TREE_MODEL (model);

#undef N_NUMBERS
}

static void add_items (void)
{
  TreeItem foo;
  int c=0, ret, nRows;

  string SQLst = "select count(*) from products";

    ret = sqlite3_prepare_v2(pDB, &SQLst[0], SQLst.length() , &pStmt, NULL);
    sqlite3_step(pStmt);
    nRows = sqlite3_column_int(pStmt, 0);

    //cout << "Rows: " << nRows << endl;

    SQLst = "select * from products";

    ret = sqlite3_prepare_v2(pDB, &SQLst[0], SQLst.length() , &pStmt, NULL);
    ret == SQLITE_OK ? cout << "SQL statement OK!" <<endl : cout << "SQL statement Failed..." <<endl;


  //g_return_if_fail (articles != NULL);

  //char **row;// = mysql_fetch_row(mRes);

  //mRes->row_count = 10;
  //cout << "i:" <<  << endl;

  //for(int i=0; i < 2; ++i)

  vector<string> tmp;
  tmp.assign(0,"");
  string str;

  //tmp.insert(tmp.begin(), "empty");
  switch(sqlite3_step(pStmt))
	{
		case SQLITE_ROW :{
            while( nRows > 0)
            {
                foo.number  = sqlite3_column_int(pStmt, 0);
                //foo.product = g_strdup((const gchar*)sqlite3_column_text(pStmt, 1));
                foo.product    = g_convert((const gchar*)sqlite3_column_text(pStmt, 1), -1, "UTF-8", "UTF-8", NULL, NULL, NULL);
                foo.prop       = g_strdup((const gchar*)sqlite3_column_text(pStmt, 2));
                foo.price      = sqlite3_column_double(pStmt, 3);
                foo.factPrice  = sqlite3_column_double(pStmt, 4);
                foo.quant      = sqlite3_column_int(pStmt, 5);

                if( (const gchar*)sqlite3_column_text(pStmt, 6) != NULL)
                    foo.factProdID = g_convert((const gchar*)(const gchar*)sqlite3_column_text(pStmt, 6), -1, "UTF-8", "UTF-8", NULL, NULL, NULL);
                else foo.factProdID = NULL;

                foo.tick = false;
                foo.miqdor = 0;

                prods.push_back(foo);
                str.assign(foo.product);
                map_prods[str].push_back(foo);
                --nRows; //str.clear();
                //cout << nRows << str << foo.prop <<endl;
                sqlite3_step(pStmt);
            }
        }break;
  		case SQLITE_DONE: {
				cout << "Execution done" <<endl;
				}break;
		default: break;
    }

  /*while( row = mysql_fetch_row(mRes) ) pre!!!
  {
    //row = mysql_fetch_row(mRes);

    foo.number  = atoi(row[0]);
    foo.product = g_strdup (row[1]);
    foo.prop    = g_strdup (row[2]);
    foo.price   = atol(row[4]);
    foo.quant   = atoi(row[3]);
    foo.factPrice = atof(row[5]);
    foo.tick = false;
    foo.miqdor = 0;
    //foo.children = NULL;
    //g_array_append_vals (articles, &foo, 1);

    prods.push_back(foo);
    map_prods[foo.product].push_back(foo);

    //map_prods[foo.product].push_back(foo.prop);

    /*if(map_prods.count(foo.product))
    {
        //tmp.push_back(foo.prop);
        map_prods[foo.product].push_back(foo.prop);

        //cout << map_prods[foo.product][0] << endl;
    }
    else
    {
     map_prods[foo.product].push_back(foo.prop);
     //cout << foo.product << foo.prop << endl;
    }/
  }*/

  //cout << map_prods["GBC"][0].number << endl;
}

static GtkTreeModel *create_items_model (void)
{
  GtkTreeStore *model = NULL;
  GtkTreeIter iter;

  /* create array */
  //articles = g_array_sized_new (FALSE, FALSE, sizeof (Item), 1);

  add_items ();

  /* create list store */
  model = gtk_tree_store_new (NUM_ITEM_COLUMNS,
                                G_TYPE_STRING,
                                G_TYPE_INT,
                                G_TYPE_STRING,//factProdID
                                G_TYPE_STRING,
                                G_TYPE_INT,
                                G_TYPE_FLOAT, G_TYPE_FLOAT,
                                G_TYPE_BOOLEAN, G_TYPE_INT,
                                G_TYPE_BOOLEAN);


  /* add items */
  map<string, vector<TreeItem> >::iterator it = map_prods.begin();
  //for (gint i = 0; i < map_prods.size()/* articles->len*/; i++)
  for( ; it != map_prods.end(); it++ )
    {
      //cout << prods[i].product << endl;
      gtk_tree_store_append (model, &iter, NULL);
      gtk_tree_store_set(model, &iter,
                         COLUMN_ITEM_PRODUCT, &(it->first[0]), /*prods[i].product,
                         /*COLUMN_ITEM_NUMBER, 0,
                         COLUMN_ITEM_PROP, NULL,
                         COLUMN_ITEM_QUANT, 0,
                         COLUMN_ITEM_PRICE, 0.0,
                         COLUMN_ITEM_FACT_PRICE, 0.0,
                         COLUMN_ITEM_PROD_TICK, FALSE,
                         COLUMN_ITEM_MIQDOR, NULL,
                         VISIBLE_COLUMN, FALSE,*/
                        -1);

     //for( gint c = 0; c < map_prods[prods[i].product].size(); c++)
     for( gint c = 0; c < map_prods[it->first].size(); c++)
     {
        GtkTreeIter child_iter;

        gtk_tree_store_append (model, &child_iter, &iter);

        gtk_tree_store_set(model, &child_iter,
          COLUMN_ITEM_NUMBER,
          //g_array_index (articles, Item, i).number
          map_prods[it->first][c].number,

          COLUMN_ITEM_FACT_ID,
          map_prods[it->first][c].factProdID,
          //COLUMN_ITEM_PRODUCT,
          //g_array_index (articles, Item, i).product
          //prods[i].product,
          COLUMN_ITEM_PROP,
          //g_array_index(articles, Item, i).prop
          map_prods[it->first][c].prop,
          COLUMN_ITEM_QUANT,
          //g_array_index(articles, Item, i).quant
          map_prods[it->first][c].quant,
          COLUMN_ITEM_PRICE,
          //g_array_index(articles, Item, i).price
          map_prods[it->first][c].price,
          COLUMN_ITEM_FACT_PRICE,
          //g_array_index(articles, Item, i).factPrice
          map_prods[it->first][c].factPrice,
          COLUMN_ITEM_PROD_TICK,
          //g_array_index(articles, Item, i).tick
          map_prods[it->first][c].tick,

          COLUMN_ITEM_MIQDOR,
          map_prods[it->first][c].miqdor,

          VISIBLE_COLUMN,
          TRUE,
          -1);
      //cout <<  g_array_index(articles, Item, i).prop << endl;
    }
   }

   //fill_prodcmb_entry(NULL);
  return GTK_TREE_MODEL (model);
}

int c = 0;

static void cell_edited (GtkCellRendererText *cell,
             const gchar         *path_string,
             const gchar         *new_text,
             gpointer             data)
{
  GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
  GtkTreeIter iter, parent_iter;
  gchar *prodName=new gchar[WORD_SIZE];
  gint prod_index=0;
  gint column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (cell), "column"));


  gtk_tree_model_get_iter (model, &iter, path);


  //g_stpcpy(gPath_str, path_string);

  vector<TreeItem>::iterator rit;

/*   tmp_pr.number = 0;
   tmp_pr.factPrice=0;
   tmp_pr.price=0;
   tmp_pr.prop = g_strdup(new_text);
   tmp_pr.quant=0;
   tmp_pr.tick=false;

   rit = find(prods.begin(), prods.end(), tmp_pr);*/
  gint i=0;

   gtk_tree_model_get(items_model, &iter, COLUMN_ITEM_NUMBER, &prod_index, -1);

   map<string, vector<TreeItem> >::iterator it = map_prods.begin();
   string str;

    /*for(; it != map_prods.end(); ++it)
    {
        //cout << it->first << " " << it->second[i].number << endl;
        for(; it->second[i].number != prod_index ; )
            {
                i++;
            }
    }*/

   if(prod_index != 0){
        gtk_tree_model_iter_parent(items_model, &parent_iter, &iter);
        gtk_tree_model_get(items_model, &parent_iter, COLUMN_ITEM_PRODUCT, &prodName, -1);

        str.assign(prodName);

            while(map_prods[str][i].number != prod_index)
            {
                i++;
            }
    bsave_db = true;
    //cout << prodName << " "<< prod_index << " " << i << " " <<map_prods[str][i].number <<endl;
  }


  switch (column)
    {
        case COLUMN_ITEM_FACT_ID:{
            prods[i].factProdID = g_strdup(new_text);
            map_prods[str][i].factProdID = g_strdup(new_text);

            gtk_tree_store_set (GTK_TREE_STORE (model), &iter, column,  map_prods[str][i].factProdID, -1);
            bsave_db = true;
        }break;

    case COLUMN_ITEM_PROP :
      {
          //printf("%d", prod_index);

        //i = gtk_tree_path_get_indices (path)[0];

        //g_array_index (articles, Item, i).prop = g_strdup(new_text);
        prods[i].prop = g_strdup(new_text);
        //c <= prods.size() ? ++c : c=0;



        /*vector<Item>::const_iterator rit;

        //tmp_pr = prods[0];
        tmp_pr.number = 2;
        tmp_pr.factPrice=0;
        tmp_pr.price=0;
        tmp_pr.prop = g_strdup(new_text);
        //tmp_pr.product="";
        tmp_pr.quant=0;
        tmp_pr.tick=false;*/


        //if(map_prods[ new_text ].size() > 1)
        {

            //rit = find(prods.begin(), prods.end(), tmp_pr);

            //if( rit != prods.end()  ) cout << rit->product << endl;
            //cout << "TEST:" << new_text << endl;

            map_prods[str][i].prop = g_strdup(new_text);

            //gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,  new_text, -1);
            gtk_tree_store_set (GTK_TREE_STORE (model), &iter, column,  map_prods[str][i].prop, -1);
            bsave_db = true;

            /*gtk_list_store_set (GTK_LIST_STORE (items_model), &iter,
                                COLUMN_ITEM_NUMBER,  rit->number,
                                COLUMN_ITEM_PRICE, rit->price,
                                COLUMN_ITEM_FACT_PRICE, rit->factPrice,
                                COLUMN_ITEM_QUANT, rit->quant,
                                COLUMN_ITEM_MIQDOR, rit->miqdor,
                                COLUMN_ITEM_PROD_TICK, rit->tick,
                                -1);*/
          //cout << tmp_pr.prop << " " << rit->number << endl;
        }
      }
      break;

    case COLUMN_ITEM_PRICE :
    {
            /*gtk_tree_model_get(items_model, &iter, COLUMN_ITEM_NUMBER, &prod_index, -1);

            i=0;
            while(map_prods[prods[prod_index-1].product][i].number != prods[prod_index-1].number)
            {
                i++;
            }*/
            bsave_db = true;
            prods[i].price = atof(new_text);
            map_prods[str][i].price = atof(new_text);

            //gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,  new_text, -1);
            gtk_tree_store_set (GTK_TREE_STORE (model), &iter, column,  map_prods[str][i].price, -1);
    } break;

    case COLUMN_ITEM_FACT_PRICE :
    {
            /*gtk_tree_model_get(items_model, &iter, COLUMN_ITEM_NUMBER, &prod_index, -1);

            i=0;
            while(map_prods[prods[prod_index-1].product][i].number != prods[prod_index-1].number)
            {
                i++;
            }*/
            bsave_db = true;
            prods[i].factPrice = atof(new_text);
            map_prods[str][i].factPrice = atof(new_text);

            //gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,  new_text, -1);
            gtk_tree_store_set (GTK_TREE_STORE (model), &iter, column,  map_prods[str][i].factPrice, -1);
    } break;

    case COLUMN_ITEM_QUANT:
    {
            /*gtk_tree_model_get(items_model, &iter, COLUMN_ITEM_NUMBER, &prod_index, -1);

            i=0;
            while(map_prods[prods[prod_index-1].product][i].number != prods[prod_index-1].number)
            {
                i++;
            }*/
            if(map_prods[str][i].quant >= 0)
                {
                    bsave_db = true;
                    prods[i].quant += atoi(new_text);
                    map_prods[str][i].quant += atoi(new_text);
                }
            else
            {
                bsave_db = true;
                prods[i].quant = atoi(new_text);
                map_prods[str][i].quant = atoi(new_text);
            }

            //gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,  new_text, -1);
            gtk_tree_store_set (GTK_TREE_STORE (model), &iter, column,  map_prods[str][i].quant, -1);
    } break;

    case COLUMN_ITEM_MIQDOR:{

          gchar *old_text;
          gboolean tick;

          //gtk_tree_model_get (model, &iter, column, &old_text, -1);
          //g_free (old_text);

          //i = gtk_tree_path_get_indices (path)[0];
          bsave_db = true;
          gtk_tree_model_get(items_model, &iter, COLUMN_ITEM_NUMBER, &prod_index, -1);

          gtk_tree_model_get (items_model, &iter, COLUMN_ITEM_PROD_TICK, &tick, -1);

          //old_text = g_strdup("0");

          //gtk_list_store_set (GTK_LIST_STORE (model), &iter, column, old_text, -1);


          prods[i].miqdor = atoi(new_text);
          //prods[prod_index-1].miqdor > 0 ? prods[prod_index-1].tick = true : prods[prod_index-1].tick = false;

            /*i=0;
            while(map_prods[prods[prod_index-1].product][i].number != prods[prod_index-1].number)
            {
                i++;
            }*/

          map_prods[str][i].miqdor = atoi(new_text);
          map_prods[str][i].miqdor > 0 ? map_prods[str][i].tick = true :
                                                                 map_prods[str][i].tick = false;
          //rit->miqdor = atoi(new_text);


          //cout << prods[prod_index-1].miqdor <<  " " << prod_index <<endl;
          //cout << path_string << endl;

          gtk_tree_store_set (GTK_TREE_STORE (model), &iter, column,  map_prods[str][i].miqdor, -1);

          gtk_tree_store_set(GTK_TREE_STORE(items_model), &iter, COLUMN_ITEM_PROD_TICK, map_prods[str][i].tick, -1);


        } break;

      case COLUMN_ITEM_PRODUCT:
          {
                bsave_db = true;

              //map_prods[prods[prod_index-1].product][i].prop = g_strdup(new_text);
              new_text = g_convert(new_text, -1, "UTF-8", "UTF-8", NULL, NULL, NULL);

              gchar *old_text;
              string tmp_str, tmp;
              map<string, vector<TreeItem> > tmp_map;

              gtk_tree_model_get(items_model, &iter, COLUMN_ITEM_PRODUCT, &old_text, -1);

              tmp_str.assign(old_text);
              g_free(old_text);

              tmp_map[tmp_str] = map_prods[tmp_str];
              map_prods.erase(map_prods.find(tmp_str));

              tmp.assign(new_text);
              map_prods[tmp] = tmp_map[tmp_str];

              /*for(i=0; i<map_prods[tmp_str].size(); i++)
                {
                    map_prods[tmp][i].product = g_strdup(new_text);
                }*/

              //cout <<  prod_index << tmp << endl;
              gtk_tree_store_set (GTK_TREE_STORE (model), &iter, column,  new_text, -1);

          } break;

      case COLUMN_ITEM_PROD_TICK: {

           //gtk_tree_model_get(items_model, &iter, COLUMN_ITEM_NUMBER, &prod_index, -1);
        } break;
    }

  delete [] prodName;
  gtk_tree_path_free (path);
}

static void
add_columns (GtkTreeView  *treeview/*,
             GtkTreeModel *items_model,
             GtkTreeModel *numbers_model*/)
{
  gint col_offset, mcase;
  GtkCellRenderer   *renderer;
  GtkTreeViewColumn *column;
  GtkTreeModel *model = gtk_tree_view_get_model (treeview);

  /* product column */
  arenderer[4] = gtk_cell_renderer_text_new ();

  //g_object_set (renderer,  "xalign", 0.0, NULL);
  //g_object_set_data (G_OBJECT (arenderer[4]), "column", GINT_TO_POINTER (COLUMN_ITEM_PRODUCT));

  g_signal_connect (arenderer[4], "edited",  G_CALLBACK (cell_edited), items_model);

  col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                                            -1, "Feature",
                                                            arenderer[4], "text",
                                                            COLUMN_ITEM_PRODUCT,
                                                            NULL);
  column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
  g_object_set_data (G_OBJECT (column), "column", GINT_TO_POINTER (COLUMN_ITEM_PRODUCT));

  gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);


  /* number column */
  renderer = gtk_cell_renderer_text_new();

  /*g_signal_connect (renderer, "edited",
                    G_CALLBACK (cell_edited), items_model);
  g_signal_connect (renderer, "editing-started",
                    G_CALLBACK (editing_started), NULL);*/
  g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (COLUMN_ITEM_NUMBER));

  col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                                            -1, "в„–",
                                                            renderer,
                                                            "text", COLUMN_ITEM_NUMBER,
                                                            "visible", VISIBLE_COLUMN,
                                                            NULL);

    column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
    g_object_set_data (G_OBJECT (column), "column", GINT_TO_POINTER (COLUMN_ITEM_NUMBER));

    /*factProdID column*/
    arenderer[5] = gtk_cell_renderer_text_new();
    g_signal_connect (arenderer[5], "edited",  G_CALLBACK (cell_edited), items_model);
    g_object_set_data (G_OBJECT (arenderer[5]), "column", GINT_TO_POINTER (COLUMN_ITEM_FACT_ID));
    col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, "ProdID", arenderer[5],
                                               "text", COLUMN_ITEM_FACT_ID,
                                               "visible", VISIBLE_COLUMN,
                                               NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
    g_object_set_data (G_OBJECT (column), "column", GINT_TO_POINTER (COLUMN_ITEM_FACT_ID));

  /* Prop yummy column */
  arenderer[0] = gtk_cell_renderer_text_new();
  /*renderer = gtk_cell_renderer_combo_new();

  g_object_set (renderer,
                "model", numbers_model,
                "text-column", COLUMN_PROP_COMBO,
                "has-entry", FALSE,
                "editable", TRUE,
                NULL);


  g_signal_connect (renderer, "edited", G_CALLBACK (cell_edited), items_model);*/
  g_signal_connect (arenderer[0], "edited",  G_CALLBACK (cell_edited), items_model);

  g_object_set_data (G_OBJECT (arenderer[0]), "column", GINT_TO_POINTER (COLUMN_ITEM_PROP));
  col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, "Product", arenderer[0],
                                               "text", COLUMN_ITEM_PROP,
                                               "visible", VISIBLE_COLUMN,
                                               NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
    g_object_set_data (G_OBJECT (column), "column", GINT_TO_POINTER (COLUMN_ITEM_PROP));

   /*col Quant*/
  arenderer[1] = gtk_cell_renderer_text_new();
  g_signal_connect (arenderer[1], "edited",  G_CALLBACK (cell_edited), items_model);
  g_object_set_data (G_OBJECT (arenderer[1]), "column", GINT_TO_POINTER (COLUMN_ITEM_QUANT));

  col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, "Quantity", arenderer[1],
                                               "text", COLUMN_ITEM_QUANT,
                                               "visible", VISIBLE_COLUMN,
                                               NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
    g_object_set_data (G_OBJECT (column), "column", GINT_TO_POINTER (COLUMN_ITEM_QUANT));

  arenderer[2] = gtk_cell_renderer_text_new();
  g_signal_connect (arenderer[2], "edited",  G_CALLBACK (cell_edited), items_model);
  //g_signal_connect(renderer, "edited", G_CALLBACK(cell_edited_cb),  items_model);
  g_object_set_data (G_OBJECT (arenderer[2]), "column", GINT_TO_POINTER (COLUMN_ITEM_PRICE));

  col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, "Price", arenderer[2],
                                               "text", COLUMN_ITEM_PRICE,
                                               "visible", VISIBLE_COLUMN,
                                               NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
    g_object_set_data (G_OBJECT (column), "column", GINT_TO_POINTER (COLUMN_ITEM_PRICE));
    mcase = 2;
    gtk_tree_view_column_set_cell_data_func(column, arenderer[2], price_cell_data_func, &mcase, NULL);

 arenderer[3] = gtk_cell_renderer_text_new();
 g_signal_connect (arenderer[3], "edited",  G_CALLBACK (cell_edited), items_model);

  g_object_set_data (G_OBJECT (arenderer[3]), "column", GINT_TO_POINTER (COLUMN_ITEM_FACT_PRICE));

  col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, "Factory Price", arenderer[3],
                                               "text", COLUMN_ITEM_FACT_PRICE,
                                               "visible", VISIBLE_COLUMN,
                                               NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
    g_object_set_data (G_OBJECT (column), "column", GINT_TO_POINTER (COLUMN_ITEM_FACT_PRICE));
    mcase = 3;
    gtk_tree_view_column_set_cell_data_func(column, arenderer[3], factPrice_cell_data_func, &mcase, NULL);

//tick mark
  renderer = gtk_cell_renderer_toggle_new();
  g_object_set (renderer, "xalign", 0.0, NULL);

  g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (COLUMN_ITEM_PROD_TICK));

  //renderer = gtk_cell_renderer_text_new();

  g_signal_connect (renderer, "toggled", G_CALLBACK (item_toggled), items_model);

  col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                                            -1, "Tick",
                                                            renderer,
                                                            "active",
                                                            COLUMN_ITEM_PROD_TICK,
                                                            "visible", VISIBLE_COLUMN,
                                                            NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
    g_object_set_data (G_OBJECT (column), "column", GINT_TO_POINTER (COLUMN_ITEM_PROD_TICK));

 renderer = gtk_cell_renderer_text_new();
 g_object_set (renderer, "editable", TRUE,  NULL);

 //g_object_set(renderer, "editable", TRUE, "xalign", 1.0, "align-set", TRUE, NULL);

 g_signal_connect (renderer, "edited",  G_CALLBACK (cell_edited), items_model);

 //g_signal_connect(renderer, "edited", G_CALLBACK(cell_edited_cb),  items_model);

 g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (COLUMN_ITEM_MIQDOR));


 col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, "Miqdor", renderer,
                                               "text", COLUMN_ITEM_MIQDOR,
                                               "visible", VISIBLE_COLUMN,
                                               NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), col_offset - 1);
    g_object_set_data (G_OBJECT (column), "column", GINT_TO_POINTER (COLUMN_ITEM_MIQDOR));
    //cout << "Offset:"<<col_offset << endl;
}

vector<string> vstr;

static void
begin_print (GtkPrintOperation *operation,
             GtkPrintContext   *context,
             gpointer           user_data)
{
  PrintData *data = (PrintData *)user_data;
  GBytes *bytes;
  int c,i;
  double height;
  gsize sz;
  guint32 flags;
  gboolean bret = TRUE;

  //GResourceLookupFlags look_flags;


  height = gtk_print_context_get_height (context) - HEADER_HEIGHT - HEADER_GAP;

  data->lines_per_page = floor (height / data->font_size);

  //bytes = g_resources_lookup_data (data->resourcename, G_RESOURCE_LOOKUP_FLAGS_NONE, NULL); //**!!
  //bret = g_resources_get_info (data->resourcename, G_RESOURCE_LOOKUP_FLAGS_NONE, &sz, &flags, NULL);
  //bret ? cout << "good"<< endl: cout << "bad"<< endl;
  //bytes == NULL ? cout<<"FAIL"<<endl:cout << data->resourcename << endl;


  //data->lines = g_strsplit ((gchar*)g_bytes_get_data (bytes, NULL), "\n", -1);
  //data->lines = g_strsplit ("akslakslks", "s", 0);

  //g_bytes_unref (bytes);


  /*i = 0;
  while (data->lines[i] != NULL)
    i++;*/
  vstr.clear();
  string str, tmp;
  map<string, vector<TreeItem> >::const_iterator it = map_prods.begin();

  for( ; it != map_prods.end(); it++ )
    {
        for(int i = 0; i < map_prods[it->first].size(); i++)
        {

          if( map_prods[it->first][i].tick) {
            str.assign(map_prods[it->first][i].prop); str.append(" ");
            str.append(map_prods[it->first][i].product); str.append(" ");

            g_ascii_dtostr( &tmp[0], -1, map_prods[it->first][i].miqdor);
            str.append(&tmp[0]);
            //cout << map_prods[prods[c].product][i].prop << endl;
            //cout << str << endl;
            vstr.push_back(str);
           }
         }
   }

  vstr.push_back("----------------------------");
  str = "Sum: ";
  g_ascii_dtostr( &tmp[0], -1, sum);
  str.append(&tmp[0]);
  vstr.push_back(str);

  data->num_lines = rows+2;
  data->num_pages = (data->num_lines - 1) / data->lines_per_page + 1;

  gtk_print_operation_set_n_pages (operation, data->num_pages);
}

static void
draw_page (GtkPrintOperation *operation,
           GtkPrintContext   *context,
           gint               page_nr,
           gpointer           user_data)
{
  PrintData *data = (PrintData *)user_data;
  cairo_t *cr;
  PangoLayout *layout;
  gint text_width, text_height;
  gdouble width;
  gint line, i;
  PangoFontDescription *desc;
  gchar *page_str = new gchar[WORD_SIZE];

  cr = gtk_print_context_get_cairo_context (context);
  width = gtk_print_context_get_width (context);

  cairo_rectangle (cr, 0, 0, width, HEADER_HEIGHT);

  cairo_set_source_rgb (cr, 0.8, 0.8, 0.8);
  cairo_fill_preserve (cr);

  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_set_line_width (cr, 1);
  cairo_stroke (cr);

  layout = gtk_print_context_create_pango_layout (context);

  desc = pango_font_description_from_string ("sans 14");
  pango_layout_set_font_description (layout, desc);
  pango_font_description_free (desc);

  pango_layout_set_text (layout, data->resourcename, -1);
  pango_layout_get_pixel_size (layout, &text_width, &text_height);

  if (text_width > width)
    {
      pango_layout_set_width (layout, width);
      pango_layout_set_ellipsize (layout, PANGO_ELLIPSIZE_START);
      pango_layout_get_pixel_size (layout, &text_width, &text_height);
    }

  cairo_move_to (cr, (width - text_width) / 2,  (HEADER_HEIGHT - text_height) / 2);
  pango_cairo_show_layout (cr, layout);

  time_t d1;
  time(&d1);
  pTime = localtime(&d1);

  //page_str = g_strdup_printf ("%d/%d", page_nr + 1, data->num_pages);
  //page_str = g_strdup_printf ("%d %d %d/%d %d", pTime->tm_mday, pTime->tm_mon, pTime->tm_year, pTime->tm_hour, pTime->tm_min);
  //page_str = asctime(pTime);
  strftime(page_str, WORD_SIZE,"Vaqt: %H:%M - Sana: %d/%m/%Y", pTime);
  cout << page_str << endl;
  pango_layout_set_text (layout, page_str, -1);
  delete [] page_str;
  //g_free (page_str);

  pango_layout_set_width (layout, -1);
  pango_layout_get_pixel_size (layout, &text_width, &text_height);
  cairo_move_to (cr, width - text_width - 4, (HEADER_HEIGHT - text_height) / 2);
  pango_cairo_show_layout (cr, layout);

  g_object_unref (layout);

  layout = gtk_print_context_create_pango_layout (context);

  desc = pango_font_description_from_string ("monospace");
  pango_font_description_set_size (desc, data->font_size * PANGO_SCALE);
  pango_layout_set_font_description (layout, desc);
  pango_font_description_free (desc);

  cairo_move_to (cr, 0, HEADER_HEIGHT + HEADER_GAP);
  line = page_nr * data->lines_per_page;


  for (i = 0; i < data->lines_per_page && line < data->num_lines; i++)
    {
      //pango_layout_set_text (layout, data->lines[line], -1);
      /*str.assign(prods[line].product);
      str.append(prods[line].prop);*/
      //cout << prods[line].product <<endl;

      pango_layout_set_text(layout,&(vstr[line][0]), -1 );
      pango_cairo_show_layout (cr, layout);
      cairo_rel_move_to (cr, 0, data->font_size);

      line++;
    }

  g_object_unref (layout);
}

static void
end_print (GtkPrintOperation *operation,
           GtkPrintContext   *context,
           gpointer           user_data)
{
  PrintData *data = (PrintData *)user_data;

  g_free (data->resourcename);
  g_strfreev (data->lines);
  g_free (data);
}

GtkWidget *
do_printing (GtkWidget *do_widget)
{
  //cout << "test" <<endl;
  GtkPrintOperation *operation;
  GtkPrintSettings *settings;
  PrintData *data;
  GError *error = NULL;

  operation = gtk_print_operation_new ();
  data = g_new0 (PrintData, 1);
  //data->resourcename = g_strdup ("/home/mdt/test.txt");

  data->font_size = 12.0;

  g_signal_connect (G_OBJECT (operation), "begin-print",
                    G_CALLBACK (begin_print), data);
  g_signal_connect (G_OBJECT (operation), "draw-page",
                    G_CALLBACK (draw_page), data);
  g_signal_connect (G_OBJECT (operation), "end-print",
                    G_CALLBACK (end_print), data);

  gtk_print_operation_set_use_full_page (operation, FALSE);
  gtk_print_operation_set_unit (operation, GTK_UNIT_POINTS);
  gtk_print_operation_set_embed_page_setup (operation, TRUE);

  settings = gtk_print_settings_new ();

  gtk_print_settings_set (settings, GTK_PRINT_SETTINGS_OUTPUT_BASENAME, "gtk-demo");
  gtk_print_operation_set_print_settings (operation, settings);

  gtk_print_operation_run (operation, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, GTK_WINDOW (win), &error);

  g_object_unref (operation);
  g_object_unref (settings);

  if (error)
    {
      GtkWidget *dialog;

      dialog = gtk_message_dialog_new (GTK_WINDOW (do_widget),
                                       GTK_DIALOG_DESTROY_WITH_PARENT,
                                       GTK_MESSAGE_ERROR,
                                       GTK_BUTTONS_CLOSE,
                                       "%s", error->message);
      g_error_free (error);

      g_signal_connect (dialog, "response",
                        G_CALLBACK (gtk_widget_destroy), NULL);

      gtk_widget_show (dialog);
    }

  return NULL;
}

void about_us( GtkWidget *widget,
            gpointer   data )
{

   gtk_show_about_dialog (NULL,
                          "program-name", prog_name,
                          "copyright", "Muqobil Dasturlar To'plami (c) 2014-?\nEvaluation edition",
                          "license", "The soft is released under the terms of GPL version 2.",
                          "version", ver,
                          "comments", "O'zbekiston, Toshkent shahri\n  muqobildasturlar@gmail.com",
                          "title", ("About Store Capture!"),
                          "authors", NULL,
                         NULL);

}
