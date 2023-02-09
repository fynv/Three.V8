using System;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Input;
using Microsoft.Win32;
using Newtonsoft.Json.Linq;
using System.Diagnostics;
using System.Threading.Tasks;
using Microsoft.VisualBasic.FileIO;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public static RoutedCommand RoutedCommandNewProject = new RoutedCommand();
        public static RoutedCommand RoutedCommandOpenProject = new RoutedCommand();
        public static RoutedCommand RoutedCommandCloseFile = new RoutedCommand();
        public static RoutedCommand RoutedCommandCloseProject = new RoutedCommand();
        public static RoutedCommand RoutedCommandSaveAll = new RoutedCommand();
        public static RoutedCommand RoutedCommandReopen = new RoutedCommand();
        public static RoutedCommand RoutedCommandReopenAll = new RoutedCommand();

        public static RoutedCommand RoutedCommandComment = new RoutedCommand();
        public static RoutedCommand RoutedCommandUpper = new RoutedCommand();
        public static RoutedCommand RoutedCommandLower = new RoutedCommand();
        public static RoutedCommand RoutedCommandFind = new RoutedCommand();
        public static RoutedCommand RoutedCommandFindNext = new RoutedCommand();
        public static RoutedCommand RoutedCommandFindPrev = new RoutedCommand();
        public static RoutedCommand RoutedCommandReplace = new RoutedCommand();
        public static RoutedCommand RoutedCommandGoto = new RoutedCommand();

        public static RoutedCommand RoutedCommandProjectSettings = new RoutedCommand();
        public static RoutedCommand RoutedCommandAPI = new RoutedCommand();

        public string cur_path = "";
        private JsonData project = new JsonData();
        private HashSet<string> target_outputs = new HashSet<string>();
        private Dictionary<string, TabItem> opened_tabs = new Dictionary<string, TabItem>();

        private List<string> RecentFiles = new List<string>();
        private List<string> RecentProjects = new List<string>();

        private ContextMenu ctxMenu_dir =null;
        private ContextMenu ctxMenu_file = null;
        private ContextMenu ctxMenu_target = null;

        private void CreateContextMenus()
        {
            ctxMenu_dir = new ContextMenu();
            {
                var item_add = new MenuItem();
                item_add.Header = "_New File";
                ctxMenu_dir.Items.Add(item_add);
                item_add.Click += (sender, e) =>
                {
                    MenuItem mi = sender as MenuItem;
                    if (mi != null)
                    {
                        ContextMenu cm = (ContextMenu)mi.Parent;
                        TreeViewItem node = (TreeViewItem)cm.PlacementTarget;
                        string dir_path = (string)node.Tag;
                        NewFile(dir_path);
                    }
                };

                var item_add_dir = new MenuItem();
                item_add_dir.Header = "_New Directory";
                ctxMenu_dir.Items.Add(item_add_dir);
                item_add_dir.Click += (sender, e) =>
                {
                    MenuItem mi = sender as MenuItem;
                    if (mi != null)
                    {
                        ContextMenu cm = (ContextMenu)mi.Parent;
                        TreeViewItem node = (TreeViewItem)cm.PlacementTarget;
                        string dir_path = (string)node.Tag;
                        NewDirectory(dir_path);
                    }
                };

                var item_delete = new MenuItem();
                item_delete.Header = "_Delete";
                ctxMenu_dir.Items.Add(item_delete);
                item_delete.Click += (sender, e) =>
                {
                    MenuItem mi = sender as MenuItem;
                    if (mi != null)
                    {
                        ContextMenu cm = (ContextMenu)mi.Parent;
                        TreeViewItem node = (TreeViewItem)cm.PlacementTarget;
                        string dir_path = (string)node.Tag;

                        if (MessageBox.Show($"Delete directory\"{dir_path}\"?", "Delete Directory", MessageBoxButton.YesNo, MessageBoxImage.Warning) == MessageBoxResult.Yes)
                        {
                            FileSystem.DeleteDirectory(dir_path, UIOption.OnlyErrorDialogs, RecycleOption.SendToRecycleBin);
                            update_cur_path();
                        }
                    }
                };
            }

            ctxMenu_file = new ContextMenu();
            {
                var item_open = new MenuItem();
                item_open.Header = "_Open";
                ctxMenu_file.Items.Add(item_open);
                item_open.Click += (sender, e) =>
                {
                    MenuItem mi = sender as MenuItem;
                    if (mi != null)
                    {
                        ContextMenu cm = (ContextMenu)mi.Parent;
                        TreeViewItem node = (TreeViewItem)cm.PlacementTarget;
                        string file_path = (string)node.Tag;
                        OpenFile(file_path);                        
                    }
                };

                var item_delete = new MenuItem();
                item_delete.Header = "_Delete";
                ctxMenu_file.Items.Add(item_delete);
                item_delete.Click += (sender, e) =>
                {
                    MenuItem mi = sender as MenuItem;
                    if (mi != null)
                    {
                        ContextMenu cm = (ContextMenu)mi.Parent;
                        TreeViewItem node = (TreeViewItem)cm.PlacementTarget;
                        string file_path = (string)node.Tag;

                        if (MessageBox.Show($"Delete file\"{file_path}\"?", "Delete File", MessageBoxButton.YesNo, MessageBoxImage.Warning) == MessageBoxResult.Yes)
                        {
                            FileSystem.DeleteFile(file_path, UIOption.OnlyErrorDialogs, RecycleOption.SendToRecycleBin);
                            update_cur_path();
                        }
                    }
                };
            }

            ctxMenu_target = new ContextMenu();
            {
                var item_run = new MenuItem();
                item_run.Header = "_Run";
                ctxMenu_target.Items.Add(item_run);
                item_run.Click += (sender, e) =>
                {
                    MenuItem mi = sender as MenuItem;
                    if (mi != null)
                    {
                        ContextMenu cm = (ContextMenu)mi.Parent;
                        ListViewItem item = (ListViewItem)cm.PlacementTarget;
                        JObject jTarget = (JObject)item.Tag;
                        RunTarget(jTarget);
                    }
                };

                var item_shortcut = new MenuItem();
                item_shortcut.Header = "Create _shortcut";
                ctxMenu_target.Items.Add(item_shortcut);
                item_shortcut.Click += (sender, e) =>
                {
                    MenuItem mi = sender as MenuItem;
                    if (mi != null)
                    {
                        ContextMenu cm = (ContextMenu)mi.Parent;
                        ListViewItem item = (ListViewItem)cm.PlacementTarget;
                        JObject jTarget = (JObject)item.Tag;
                        CreateTargetShortcut(jTarget);
                    }
                };

                var item_edit = new MenuItem();
                item_edit.Header = "_Edit";
                ctxMenu_target.Items.Add(item_edit);
                item_edit.Click += (sender, e) =>
                {
                    MenuItem mi = sender as MenuItem;
                    if (mi != null)
                    {
                        ContextMenu cm = (ContextMenu)mi.Parent;
                        ListViewItem item = (ListViewItem)cm.PlacementTarget;
                        JObject jTarget = (JObject)item.Tag;
                        EditTarget(jTarget);
                    }
                };


                var item_remove = new MenuItem();
                item_remove.Header = "_Remove";
                ctxMenu_target.Items.Add(item_remove);
                item_remove.Click += (sender, e) =>
                {
                    MenuItem mi = sender as MenuItem;
                    if (mi != null)
                    {
                        ContextMenu cm = (ContextMenu)mi.Parent;
                        ListViewItem item = (ListViewItem)cm.PlacementTarget;
                        JObject jTarget = (JObject)item.Tag;
                        RemoveTarget(jTarget);
                    }
                };

            }

        }

        private void UpdateRecentFiles()
        {
            menu_recent_files.Items.Clear();
            if (RecentFiles.Count<1)
            {
                MenuItem item = new MenuItem();
                item.IsEnabled = false;
                item.Header = "None";
                menu_recent_files.Items.Add(item);
                return;
            }

            for (int i =0; i< RecentFiles.Count; i++)
            {
                string filename = RecentFiles[i];
                MenuItem item = new MenuItem();
                item.Header = $"{i+1} {filename}";                
                item.Click += (sender, e) =>
                {
                    OpenFile(filename);
                };
                menu_recent_files.Items.Add(item);
            }            
        }

        private void LoadRecentFiles()
        {
            RecentFiles.Clear();
            const string reg_path = "HKEY_CURRENT_USER\\Software\\GameDev\\recent_files";
            for (int i = 0; i < 10; i++)
            {
                string key = i.ToString();
                string filename = (string)Registry.GetValue(reg_path, key, "");
                if (filename== null || filename == "") break;
                RecentFiles.Add(filename);
            }
            UpdateRecentFiles();
        }

        private void SaveRecentFiles()
        {            
            const string reg_path = "HKEY_CURRENT_USER\\Software\\GameDev\\recent_files";
            for (int i=0; i<RecentFiles.Count; i++)
            {
                string key = i.ToString();
                string filename = RecentFiles[i];
                Registry.SetValue(reg_path, key, filename);
            }
        }

        private void AddRecentFile(string file_path)
        {
            RecentFiles.Insert(0, file_path);
            for (int i = 1; i < RecentFiles.Count; i++)
            {
                if (RecentFiles[i] == file_path || i >= 10)
                {
                    RecentFiles.RemoveAt(i);
                    i--;
                }
            }
            UpdateRecentFiles();
            SaveRecentFiles();
        }

        private void UpdateRecentProjects()
        {
            menu_recent_projects.Items.Clear();
            if (RecentProjects.Count < 1)
            {
                MenuItem item = new MenuItem();
                item.IsEnabled = false;
                item.Header = "None";
                menu_recent_projects.Items.Add(item);
                return;
            }

            for (int i = 0; i < RecentProjects.Count; i++)
            {
                string filename = RecentProjects[i];
                string path = Path.GetDirectoryName(filename);
                MenuItem item = new MenuItem();
                item.Header = $"{i + 1} {filename}";                
                item.Click += (sender, e) =>
                {                    
                    change_path(path);
                };
                menu_recent_projects.Items.Add(item);
            }
        }

        private void LoadRecentProjects()
        {
            RecentProjects.Clear();
            const string reg_path = "HKEY_CURRENT_USER\\Software\\GameDev\\recent_projects";
            for (int i = 0; i < 10; i++)
            {
                string key = i.ToString();
                string filename = (string)Registry.GetValue(reg_path, key, "");
                if (filename == null || filename == "") break;
                RecentProjects.Add(filename);
            }
            UpdateRecentProjects();
        }

        private void SaveRecentProjects()
        {
            const string reg_path = "HKEY_CURRENT_USER\\Software\\GameDev\\recent_projects";
            for (int i = 0; i < RecentProjects.Count; i++)
            {
                string key = i.ToString();
                string filename = RecentProjects[i];
                Registry.SetValue(reg_path, key, filename);
            }
        }

        private void AddRecentProject(string file_path)
        {
            RecentProjects.Insert(0, file_path);
            for (int i = 1; i < RecentProjects.Count; i++)
            {
                if (RecentProjects[i] == file_path || i >= 10)
                {
                    RecentProjects.RemoveAt(i);
                    i--;
                }
            }
            UpdateRecentProjects();
            SaveRecentProjects();
        }

        private void create_default_project()
        {
            project.filename = $"{cur_path}\\project.json";

            if (!File.Exists(project.filename))
            {
                string project_name = Path.GetFileName(cur_path);
                project.data = new JObject();
                project.data["project_name"] = project_name;
                project.Save();
            }
        }

        private StackPanel create_item(string name, string icon_name, int icon_size = 24)
        {
            var panel = new StackPanel();
            panel.Orientation = Orientation.Horizontal;

            var icon_image = new BitmapImage(new Uri($"pack://application:,,,/Icons/{icon_name}"));
            var icon = new Image();
            icon.Source = icon_image;
            RenderOptions.SetBitmapScalingMode(icon, BitmapScalingMode.HighQuality);
            icon.Width = icon_size;
            icon.Height = icon_size;
            panel.Children.Add(icon);

            var label = new TextBlock();            
            label.VerticalAlignment = VerticalAlignment.Center;            
            label.Text = name;
            label.Margin = new Thickness(5.0);
            panel.Children.Add(label);
            return panel;
        }
        
        private void update_dir(ItemsControl item, string path)
        {
            item.Items.Clear();
            if (path == "") return;


            var list_dirs = Directory.GetDirectories(path);
            foreach (var dir in list_dirs)
            {
                string dir_name = Path.GetFileName(dir);
                var subitem = new TreeViewItem();
                subitem.Header = create_item(dir_name, "folder.png");
                subitem.Tag = dir;
                subitem.ContextMenu = ctxMenu_dir;
                item.Items.Add(subitem);
                update_dir(subitem, dir);
            }


            var list_files = Directory.GetFiles(path);
            foreach (var file in list_files)
            {
                string file_name = Path.GetFileName(file);
                string ext = Path.GetExtension(file_name);
                if (ext == ".js")
                {
                    string rel_path = file.Substring(cur_path.Length+1);
                    if (target_outputs.Contains(rel_path)) continue;
                }

                string icon_name = "doc.png";
                if (ext == ".js") icon_name = "js.png";
                if (ext == ".xml") icon_name = "xml.png";
                if (ext == ".json") icon_name = "json.png";

                var subitem = new TreeViewItem();
                subitem.Header = create_item(file_name, icon_name);
                subitem.Tag = file;
                subitem.ContextMenu = ctxMenu_file;

                subitem.MouseDoubleClick += (sender, e) =>
                {
                    OpenFile(file);
                };

                item.Items.Add(subitem);

            }
        } 
        
        private void update_targets()
        {
            lst_targets.Items.Clear();
            target_outputs.Clear();
            JObject obj_proj = (JObject)project.data;
            if (obj_proj != null && obj_proj.ContainsKey("targets"))
            {                
                var jTargets = (JArray)obj_proj["targets"];                

                foreach (JObject jTarget in jTargets)
                {
                    string name = jTarget["name"].ToString();
                    var subitem = new ListViewItem();
                    subitem.Content = create_item(name, "target.png", 36);
                    subitem.Tag = jTarget;
                    subitem.ContextMenu = ctxMenu_target;
                    subitem.MouseDoubleClick += (sender,e) =>
                    {
                        RunTarget(jTarget);
                    };

                    lst_targets.Items.Add(subitem);
                    target_outputs.Add(jTarget["output"].ToString());
                }
            }            

        }

        private void update_cur_path()
        {
            if (cur_path == "")
            {
                project.Clear();
                this.Title = "GameDev";
            }
            else
            {                
                if (!File.Exists($"{cur_path}\\project.json"))
                {
                    create_default_project();
                }
                else 
                {
                    project.filename = $"{cur_path}\\project.json";
                    project.Load();
                    AddRecentProject(project.filename);
                }                
                this.Title = $"GameDev - {project.data["project_name"]}";
            }
            update_targets();            
            update_dir(tree_files, cur_path);            
        }

        private void change_path(string path)
        {
            const string reg_path = "HKEY_CURRENT_USER\\Software\\GameDev";            
            Registry.SetValue(reg_path, "cur_path", path);
            cur_path = path;
            update_cur_path();

        }

        public MainWindow()
        {
            InitializeComponent();
            CreateContextMenus();

            const string reg_path = "HKEY_CURRENT_USER\\Software\\GameDev";
            string path = (string)Registry.GetValue(reg_path, "cur_path", "");
            if (File.Exists($"{path}\\project.json"))
            {
                cur_path = path;
                update_cur_path();
            }

            LoadRecentFiles();
            LoadRecentProjects();
        }

        private bool no_update = false;
        private bool in_background = false;

        public void AppDeactivated()
        {
            in_background = true;
        }

        public void AppActivated()
        {
            if (in_background && !no_update)
            {
                in_background = false;
                update_cur_path();
            }
        }

        private void TabItem_PreviewMouseMove(object sender, MouseEventArgs e)
        {
            if (!(e.Source is TextBlock tb))
            {  
                return;
            }

            TabItem tabItem = ((StackPanel)tb.Parent).Parent as TabItem;
            if (tabItem == null) return;

            if (Mouse.PrimaryDevice.LeftButton == MouseButtonState.Pressed)
            {
                DragDrop.DoDragDrop(tabItem, tabItem, DragDropEffects.All);
            }
        }

        private void TabItem_Drop(object sender, DragEventArgs e)
        {
            if (e.Source is TextBlock tb1 &&
                e.Data.GetData(typeof(TabItem)) is TabItem tabItemSource)               
            {
                TabItem tabItemTarget = (TabItem)((StackPanel)tb1.Parent).Parent;               

                if (!tabItemTarget.Equals(tabItemSource) &&
                tabItemTarget.Parent is TabControl tabControl)
                {
                    int targetIndex = tabControl.Items.IndexOf(tabItemTarget);

                    tabControl.Items.Remove(tabItemSource);
                    tabControl.Items.Insert(targetIndex, tabItemSource);
                    tabItemSource.IsSelected = true;
                }
            }
            
        }

        protected override async void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            if (TabCtrl_Editor.Items.Count > 0)
            {
                e.Cancel = true;
                while (TabCtrl_Editor.Items.Count>0)
                {
                    TabItem tabItem = (TabItem)TabCtrl_Editor.Items[0];
                    bool closed = await CloseTab(tabItem);                    
                    if (!closed) return;
                }
                _ = Dispatcher.BeginInvoke(new Action(() => Close()));
                return;
            }

            base.OnClosing(e);
        }

        private void CommandNewProject(object sender, ExecutedRoutedEventArgs e)
        {
            var dialog = new SaveFileDialog();
            dialog.Filter = "Project|project.json";
            dialog.FileName = "project.json";
            if (dialog.ShowDialog() != true) return;

            cur_path = Path.GetDirectoryName(dialog.FileName);
            create_default_project();

            change_path(cur_path);
        }

        private void CommandOpenProject(object sender, ExecutedRoutedEventArgs e)
        {
            var dialog = new OpenFileDialog();
            dialog.Filter = "Project|project.json";
            dialog.FileName = "project.json";
            if (dialog.ShowDialog() != true) return;

            string path = Path.GetDirectoryName(dialog.FileName);
            change_path(path);
        }

        private void NewFile(string path_dir)
        {
            var dialog = new DlgNewFile(this);
            if (dialog.ShowDialog() != true) return;

            string path = $"{path_dir}\\{dialog.filename}";
            if (!File.Exists(path))
            {
                if (dialog.typename == "js")
                {
                    File.WriteAllText(path, "");
                }
                else if (dialog.typename == "xml")
                {
                    File.WriteAllText(path, "<?xml version=\"1.0\" ?>\n<document>\n\t<scene>\n\t</scene>\n</document>\n");
                }
                else if (dialog.typename == "json")
                {
                    File.WriteAllText(path, "{}");
                }
                update_cur_path();
            }

            OpenFile(path);

        }

        private void NewDirectory(string path_dir)
        {
            var dialog = new DlgNewDir(this);
            if (dialog.ShowDialog() != true) return;

            string path = $"{path_dir}\\{dialog.filename}";
            if (!Directory.Exists(path))
            {
                Directory.CreateDirectory(path);
                update_cur_path();
            }
        }

        private void CommandNewFile(object sender, ExecutedRoutedEventArgs e)
        {
            NewFile(cur_path);           
        }

        private void CommandOpenFile(object sender, ExecutedRoutedEventArgs e)
        {
            var dialog = new OpenFileDialog();
            dialog.Filter = "All Supported(*.js;*.json;*.xml)|*.js;*.json;*.xml|JavaScript(*.js)|*.js|JSON(*.json)|*.json|XML(*.xml)|*.xml";
            if (dialog.ShowDialog() != true) return;
            OpenFile(dialog.FileName);            
        }

        private async Task<bool> CloseTab(TabItem tabItem)
        {
            var editor = tabItem.Content as EditorBase;
            if (editor != null)
            {
                bool closed = await editor.doc_close();
                if (closed)
                {
                    editor.cleanup();
                    string filepath = (string)tabItem.Tag;
                    TabCtrl_Editor.Items.Remove(tabItem);
                    opened_tabs.Remove(filepath);
                    return true;
                }
            }
            return false;
        }

        
        private void CommandCloseFile(object sender, ExecutedRoutedEventArgs e)
        {
            int idx = TabCtrl_Editor.SelectedIndex;
            if (idx>=0)
            {
                CloseTab((TabItem)TabCtrl_Editor.Items[idx]);
            }
        }

        private void CommandCloseProject(object sender, ExecutedRoutedEventArgs e)
        {            
            change_path("");
        }

        private void SetDirty()
        {
            if (project.data == null) return;
            JObject obj_proj = (JObject)project.data;
            if (!obj_proj.ContainsKey("targets")) return;
            JArray targets = (JArray)obj_proj["targets"];
            foreach(JObject target in targets)
            {
                target["dirty"] = true;
            }
            project.Save();
        }

        private async void SaveTab(TabItem tabItem)
        {
            var editor = tabItem.Content as Editor;
            if (editor != null)
            {
                await editor.doc_save();
                string filepath = (string)tabItem.Tag;
                string ext = Path.GetExtension(filepath);
                string filename = Path.GetFileName(filepath);

                if (filepath.StartsWith(cur_path))
                {
                    if (filename == "project.json")
                    {
                        update_cur_path();
                    }
                    if (ext == ".js")
                    {
                        SetDirty();
                    }
                }
            }
        }

        private void CommandSave(object sender, ExecutedRoutedEventArgs e)
        {
            int idx = TabCtrl_Editor.SelectedIndex;
            if (idx >= 0)
            {
                var tabItem = (TabItem)TabCtrl_Editor.Items[idx];
                SaveTab(tabItem);                
            }
        }

        private async void SaveAsTab(TabItem tabItem)
        {
            var editor = tabItem.Content as Editor;
            if (editor != null)
            {
                string new_path = await editor.doc_save_as();
                if (new_path != "")
                {
                    opened_tabs.Remove((string)tabItem.Tag);
                    opened_tabs[new_path] = tabItem;
                    tabItem.Tag = new_path;
                    StackPanel header = (StackPanel)tabItem.Header;
                    TextBlock text = (TextBlock)header.Children[0];
                    text.Text = Path.GetFileName(new_path);
                    update_cur_path();
                }
            }

        }

        private void CommandSaveAs(object sender, ExecutedRoutedEventArgs e)
        {
            int idx = TabCtrl_Editor.SelectedIndex;
            if (idx >= 0)
            {
                var tabItem = (TabItem)TabCtrl_Editor.Items[idx];
                SaveAsTab(tabItem);
            }
        }

        private void CommandSaveAll(object sender, ExecutedRoutedEventArgs e)
        {
            foreach (TabItem tabItem in TabCtrl_Editor.Items)
            {
                SaveTab(tabItem);
            }
        }

        private async void ReopenTab(TabItem tabItem)
        {
            var editor = tabItem.Content as Editor;
            if (editor != null)
            {
                await editor.doc_fresh();                
            }
        }

        private void CommandReopen(object sender, ExecutedRoutedEventArgs e)
        {
            int idx = TabCtrl_Editor.SelectedIndex;
            if (idx >= 0)
            {
                var tabItem = (TabItem)TabCtrl_Editor.Items[idx];
                ReopenTab(tabItem);
            }

        }

        private void CommandReopenAll(object sender, ExecutedRoutedEventArgs e)
        {
            foreach (TabItem tabItem in TabCtrl_Editor.Items)
            {
                ReopenTab(tabItem);
            }
        }

        private void CommandExit(object sender, ExecutedRoutedEventArgs e)
        {
            Close();
        }

        private void CommandUndo(object sender, ExecutedRoutedEventArgs e)
        {
            int idx = TabCtrl_Editor.SelectedIndex;
            if (idx >= 0)
            {
                var tabItem = (TabItem)TabCtrl_Editor.Items[idx];
                var editor = tabItem.Content as Editor;
                if (editor != null)
                {
                    editor.undo();
                }
            }
        }

        private void CommandRedo(object sender, ExecutedRoutedEventArgs e)
        {
            int idx = TabCtrl_Editor.SelectedIndex;
            if (idx >= 0)
            {
                var tabItem = (TabItem)TabCtrl_Editor.Items[idx];
                var editor = tabItem.Content as Editor;
                if (editor != null)
                {
                    editor.redo();
                }
            }
        }

        private void CommandComment(object sender, ExecutedRoutedEventArgs e)
        {
            int idx = TabCtrl_Editor.SelectedIndex;
            if (idx >= 0)
            {
                var tabItem = (TabItem)TabCtrl_Editor.Items[idx];
                var editor = tabItem.Content as Editor;
                if (editor != null)
                {
                    editor.comment();
                }
            }
        }

        private void CommandUpper(object sender, ExecutedRoutedEventArgs e)
        {
            int idx = TabCtrl_Editor.SelectedIndex;
            if (idx >= 0)
            {
                var tabItem = (TabItem)TabCtrl_Editor.Items[idx];
                var editor = tabItem.Content as Editor;
                if (editor != null)
                {
                    editor.upper();
                }
            }
        }

        private void CommandLower(object sender, ExecutedRoutedEventArgs e)
        {
            int idx = TabCtrl_Editor.SelectedIndex;
            if (idx >= 0)
            {
                var tabItem = (TabItem)TabCtrl_Editor.Items[idx];
                var editor = tabItem.Content as Editor;
                if (editor != null)
                {
                    editor.lower();
                }
            }
        }

        private void CommandFind(object sender, ExecutedRoutedEventArgs e)
        {
            int idx = TabCtrl_Editor.SelectedIndex;
            if (idx >= 0)
            {
                var tabItem = (TabItem)TabCtrl_Editor.Items[idx];
                var editor = tabItem.Content as Editor;
                if (editor != null)
                {
                    editor.find();
                }
            }
        }

        private void CommandFindNext(object sender, ExecutedRoutedEventArgs e)
        {
            int idx = TabCtrl_Editor.SelectedIndex;
            if (idx >= 0)
            {
                var tabItem = (TabItem)TabCtrl_Editor.Items[idx];
                var editor = tabItem.Content as Editor;
                if (editor != null)
                {
                    editor.findnext();
                }
            }
        }

        private void CommandFindPrev(object sender, ExecutedRoutedEventArgs e)
        {
            int idx = TabCtrl_Editor.SelectedIndex;
            if (idx >= 0)
            {
                var tabItem = (TabItem)TabCtrl_Editor.Items[idx];
                var editor = tabItem.Content as Editor;
                if (editor != null)
                {
                    editor.findprev();
                }
            }
        }

        private void CommandReplace(object sender, ExecutedRoutedEventArgs e)
        {
            int idx = TabCtrl_Editor.SelectedIndex;
            if (idx >= 0)
            {
                var tabItem = (TabItem)TabCtrl_Editor.Items[idx];
                var editor = tabItem.Content as Editor;
                if (editor != null)
                {
                    editor.replace();
                }
            }
        }

        private void CommandGoto(object sender, ExecutedRoutedEventArgs e)
        {
            int idx = TabCtrl_Editor.SelectedIndex;
            if (idx >= 0)
            {
                var tabItem = (TabItem)TabCtrl_Editor.Items[idx];
                var editor = tabItem.Content as Editor;
                if (editor != null)
                {
                    editor.gotoline();
                }
            }
        }


        private void CommandProjectSettings(object sender, ExecutedRoutedEventArgs e)
        {
            if (project.data == null) return;

            var dlg = new DlgProjectSettings(this, (JObject)project.data);
            if (dlg.ShowDialog() == true)
            {
                project.Save();
                update_cur_path();
            }
        }

        private void CommandHelp(object sender, ExecutedRoutedEventArgs e)
        {
            string local_path = Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName);
            string help_path = $"{local_path}/help/index.html";
           
            TabItem item = NewTabItem(help_path, "Help");
            if (item.Content == null)
            {
                HelpPage help_page = new HelpPage(help_path);
                item.Content = help_page;
            }
            else
            {
                HelpPage help_page = item.Content as HelpPage;
                if (help_page != null)
                {
                    help_page.Goto(help_path);
                }
            }
        }

        private void CommandAPIDoc(object sender, ExecutedRoutedEventArgs e)
        {
            string local_path = Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName);
            string help_path = $"{local_path}/help/api/index.html";

            TabItem item = NewTabItem(help_path, "Help");
            if (item.Content == null)
            {
                HelpPage help_page = new HelpPage(help_path);
                item.Content = help_page;
            }
            else
            {
                HelpPage help_page = item.Content as HelpPage;
                if (help_page != null)
                {
                    help_page.Goto(help_path);
                }
            }
        }

        private void AddTarget()
        {
            var dlg = new DlgEditTarget(this, null, cur_path);
            if (dlg.ShowDialog() == true)
            {
                JObject obj_proj = (JObject)project.data;
                JArray jTargets;
                if (!obj_proj.ContainsKey("targets"))
                {
                    jTargets = new JArray();
                    obj_proj["targets"] = jTargets;
                }
                else
                {
                    jTargets = (JArray)obj_proj["targets"];
                }
                jTargets.Add(dlg.jTarget);

                string path_in = $"{cur_path}\\{dlg.jTarget["input"]}";

                if (!File.Exists(path_in))
                {
                    File.WriteAllText(path_in, "");
                }

                project.Save();
                update_cur_path();
                lst_targets.SelectedIndex = lst_targets.Items.Count - 1;

                OpenJavaScript(path_in);

            }
        }

        private void btn_add_target_Click(object sender, RoutedEventArgs e)
        {
            if (project.data == null) return;
            AddTarget();
        }

        private void RemoveTarget(JObject jTarget)
        {
            if (MessageBox.Show($"Remove target\"{jTarget["name"]}\"?", "Remove Target", MessageBoxButton.YesNo, MessageBoxImage.Warning) == MessageBoxResult.Yes)
            {
                jTarget.Remove();
                project.Save();
                update_cur_path();
            }
        }

        private void btn_remove_target_Click(object sender, RoutedEventArgs e)
        {
            if (project.data == null) return;
            int idx = lst_targets.SelectedIndex;
            if (idx == -1) return;
            var item = (ListViewItem)lst_targets.Items[idx];
            JObject jTarget = (JObject)item.Tag;
            RemoveTarget(jTarget);
        }

        private void EditTarget(JObject jTarget)
        {
            var dlg = new DlgEditTarget(this, jTarget, cur_path);
            if (dlg.ShowDialog() == true)
            {
                project.Save();
                update_cur_path();
            }

        }

        private void btn_edit_target_Click(object sender, RoutedEventArgs e)
        {
            if (project.data == null) return;
            int idx = lst_targets.SelectedIndex;
            if (idx == -1) return;
            var item = (ListViewItem)lst_targets.Items[idx];
            JObject jTarget = (JObject)item.Tag;
            EditTarget(jTarget);
        }

        private int GetTargetIndex(JObject jTarget)
        {
            JObject obj_proj = (JObject)project.data;
            var jTargets = (JArray)obj_proj["targets"];
            for (int idx=0; idx<jTargets.Count; idx++)
            {
                JObject t = (JObject)jTargets[idx];
                if (t == jTarget) return idx;
            }
            return -1;
        }

        private Thickness thickness_zero = new Thickness(0);
        private FontFamily font_courier = new FontFamily("Courier New");
        private SolidColorBrush brush_red = new SolidColorBrush(Colors.Red);
        private SolidColorBrush brush_blue = new SolidColorBrush(Colors.Blue);

        private void console_log(string str_line, Brush brush = null)
        {
            var line = new TextBox();
            line.BorderThickness = thickness_zero;
            line.IsReadOnly = true;
            line.FontFamily = font_courier;
            if (brush != null)
            {
                line.Foreground = brush;
            }
            line.Text = str_line;
            console.Children.Add(line);            
        }

        private void console_std(string str, string tag)
        {
            string[] lines = str.Split(new char[] { '\n' });
            foreach (string str_line in lines)
            {
                console_log($"{tag}: {str_line}");
            }
            console_scroll.ScrollToBottom();
        }

        private void console_err(string str, string tag)
        {
            string[] lines = str.Split(new char[] { '\n' });
            foreach (string str_line in lines)
            {
                console_log($"{tag}: {str_line}", brush_red);
            }
            console_scroll.ScrollToBottom();
        }

        private void RunTarget(JObject jTarget)
        {
            bool dirty = (bool)jTarget["dirty"];
            if (dirty)
            {
                Directory.SetCurrentDirectory(cur_path);

                string input = (string)jTarget["input"];
                string output = (string)jTarget["output"];

                File.Delete(output);

                no_update = true;

                Process proc = new Process();
                proc.StartInfo.FileName = "cmd.exe";
                proc.StartInfo.Arguments = $"/C \"rollup.cmd {input} --file {output}\"";
                proc.StartInfo.UseShellExecute = false;
                proc.StartInfo.RedirectStandardError = true;
                proc.StartInfo.StandardErrorEncoding = Encoding.UTF8;
                proc.Start();
                proc.WaitForExit();

                console_log("running rollup.js:");
                string line;
                while ((line = proc.StandardError.ReadLine()) != null)
                {
                    line = Regex.Replace(line, "[\u001b\u009b][[()#;?]*(?:[0-9]{1,4}(?:;[0-9]{0,4})*)?[0-9A-ORZcf-nqry=><]", "");
                    console_log(line, brush_blue);
                }                

                if (!File.Exists(output))
                {
                    console_log("Bundling failed!");
                    console_log("");
                    jTarget["dirty"] = true;
                }
                else
                {
                    console_log("Bundling succeeded!");
                    console_log("");                    
                    jTarget["dirty"] = false;
                }
                console_scroll.ScrollToBottom();                
                project.Save();

                no_update = false;
            }

            int idx = GetTargetIndex(jTarget);
            string Location = Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName);
            Process.Start($"{Location}\\GamePlayer.exe", $"\"{project.filename}\" \"{idx}\"");
        }        

        private void CreateTargetShortcut(JObject jTarget)
        {
            int idx = GetTargetIndex(jTarget);
            string Location = Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName);

            var shell = new IWshRuntimeLibrary.WshShell();
            var shDesktop = (object)"Desktop";
            var shortcutAddress = (string)shell.SpecialFolders.Item(ref shDesktop) + "\\" + jTarget["name"] + ".lnk";
            var shortcut = (IWshRuntimeLibrary.IWshShortcut)shell.CreateShortcut(shortcutAddress);
            shortcut.Description = "Shortcut for Three.V8 app " + jTarget["name"];
            shortcut.TargetPath = $"{Location}\\GamePlayer.exe";
            shortcut.Arguments = $"\"{project.filename}\" \"{idx}\"";
            shortcut.Save();

            MessageBox.Show($"Created shortcut for target \"{jTarget["name"]}\"");
        }

        private TabItem NewTabItem(string filepath, string name)
        {
            if (opened_tabs.ContainsKey(filepath))
            {
                TabItem item = opened_tabs[filepath];
                TabCtrl_Editor.SelectedItem = item;
                return item;
            }

            TabItem tabItem = new TabItem();
            var header = new StackPanel();
            header.Orientation = Orientation.Horizontal;
            var text = new TextBlock();
            text.Text = name;
            header.Children.Add(text);
            var btn = new Button();
            btn.Background = Brushes.Transparent;
            btn.BorderBrush = Brushes.Transparent;
            btn.Content = "🞨";
            btn.Padding = new Thickness(2.0, 0.0, 2.0, 0.0);
            header.Children.Add(btn);
            tabItem.Header = header;
            TabCtrl_Editor.Items.Add(tabItem);
            opened_tabs[filepath] = tabItem;
            TabCtrl_Editor.SelectedItem = tabItem;
            tabItem.Tag = filepath;

            btn.Click += async (sender, e) =>
            {
                CloseTab(tabItem);

            };

            return tabItem;

        }

        private void OpenJavaScript(string file_path)
        {  
            string filename = Path.GetFileName(file_path);
            TabItem item = NewTabItem(file_path, filename);
            if (item.Content == null)
            {
                JSEditor editor = new JSEditor(file_path);
                item.Content = editor;
            }
            else
            {
                ReopenTab(item);
            }
        }

        private void OpenJSON(string file_path)
        {
            string filename = Path.GetFileName(file_path);
            TabItem item = NewTabItem(file_path, filename);
            if (item.Content == null)
            {
                JsonEditor editor = new JsonEditor(file_path);
                item.Content = editor;
            }
            else
            {
                ReopenTab(item);
            }
        }

        private void OpenXML(string file_path)
        {
            string filename = Path.GetFileName(file_path);
            TabItem item = NewTabItem(file_path, filename);
            if (item.Content == null)
            {
                XMLEditor editor = new XMLEditor(file_path, cur_path, (str) =>{
                    console_std(str, filename);
                }, (str) =>{
                    console_err(str, filename);
                });
                item.Content = editor;
            }
            else
            {
                ReopenTab(item);
            }
        }

        private void OpenFile(string file_path)
        {            
            string ext = Path.GetExtension(file_path);
            if (ext == ".js")
            {
                OpenJavaScript(file_path);         
            }
            else if (ext == ".json")
            {
                OpenJSON(file_path);                
            }
            else if (ext == ".xml")
            {
                OpenXML(file_path);                
            }
            else
            {
                return;
            }

            AddRecentFile(file_path);

        }

        private void menu_clear_console_Click(object sender, RoutedEventArgs e)
        {
            console.Children.Clear();
        }

        private void menu_new_file_Click(object sender, RoutedEventArgs e)
        {
            NewFile(cur_path);
        }

        private void menu_add_target_Click(object sender, RoutedEventArgs e)
        {
            AddTarget();
        }
    }
}
;