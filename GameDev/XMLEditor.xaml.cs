using System;
using System.Text;
using System.IO;
using System.Timers;
using System.Diagnostics;
using System.Windows.Controls;
using System.Threading.Tasks;
using System.Collections.Generic;
using Newtonsoft.Json;
using System.Windows;
using Microsoft.Win32;
using CLRBinding;
using Newtonsoft.Json.Linq;
using System.Windows.Media.Imaging;
using System.Windows.Media;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for XMLEditor.xaml
    /// </summary>
    public partial class XMLEditor : UserControl, Editor
    {
        private bool changed_cache = false;
        private string text_cache = "";
        private int cur_tab = 0;

        private string file_path;

        private CGLControl glControl = null;
        private CGamePlayer game_player = null;

        private JObject index = null;
        private Dictionary<string, TreeViewItem> TreeItemMap = new Dictionary<string, TreeViewItem>();

        private ContextMenu ctxMenu;

        public XMLEditor(string file_path, string resource_root, PrintCallback print_std, PrintCallback print_err)
        {
            InitializeComponent();
            this.file_path = file_path;

            glControl = new CGLControl();
            glControl.SetFramerate(60.0f);
            glControl.Paint += GLControl_Paint;
            glControl.Dock = System.Windows.Forms.DockStyle.Fill;
            glControl.MouseDown += GLControl_MouseDown;
            glControl.MouseUp += GLControl_MouseUp;
            glControl.MouseMove += GLControl_MouseMove;
            glControl.MouseWheel += GLControl_MouseWheel;
            glControl.KeyPress += GLControl_KeyChar;
            glControl.ControlKey += GLControl_ControlKey;
            wf_host.Child = glControl;

            string exe_name = Process.GetCurrentProcess().ProcessName;
            game_player = new CGamePlayer(exe_name, glControl);
            game_player.SetPrintCallbacks(print_std, print_err);
            game_player.AddUserMessageHandler("index_loaded", index_loaded);
            game_player.AddUserMessageHandler("object_picked", object_picked);
            game_player.AddUserMessageHandler("object_created", object_created);
            game_player.AddUserMessageHandler("object_removed", object_removed);

            string local_path = Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName);
            game_player.LoadScript($"{local_path}\\xmleditor\\bundle_index.js", resource_root);
            
            webView.Source = new Uri($"{local_path}/editor/xml_editor.html");
            webView.NavigationCompleted += (sender, e) =>
            {
                _doc_open(file_path);
            };

            ctxMenu = new ContextMenu();
            var item_remove = new MenuItem();
            item_remove.Header = "Remove";
            ctxMenu.Items.Add(item_remove);
            item_remove.Click += remove_Click;
        }

        public void cleanup()
        {
            glControl.MakeCurrent();
            game_player.Dispose();
            game_player = null;
        }

        private void GLControl_Paint(object sender, System.Windows.Forms.PaintEventArgs e)
        {
            if (game_player == null) return;
            game_player.Draw(glControl.Width, glControl.Height);
        }

        private MouseEventArgs convert_args(System.Windows.Forms.MouseEventArgs e)
        {
            int button = -1;
            if (e.Button == System.Windows.Forms.MouseButtons.Left)
            {
                button = 0;
            }
            else if (e.Button == System.Windows.Forms.MouseButtons.Middle)
            {
                button = 1;
            }
            else if (e.Button == System.Windows.Forms.MouseButtons.Right)
            {
                button = 2;
            }
            else if (e.Button == System.Windows.Forms.MouseButtons.XButton1)
            {
                button = 3;
            }
            else if (e.Button == System.Windows.Forms.MouseButtons.XButton2)
            {
                button = 4;
            }

            MouseEventArgs args;
            args.button = button;
            args.clicks = e.Clicks;
            args.delta = e.Delta;
            args.x = e.X;
            args.y = e.Y;
            return args;
        }

        private Timer press_timer = null;
        private int x_down;
        private int y_down;

        private void GLControl_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (game_player == null) return;
            glControl.Focus();
            glControl.MakeCurrent();
            game_player.OnMouseDown(convert_args(e));

            if (e.Button == System.Windows.Forms.MouseButtons.Left)
            {
                x_down = e.X;
                y_down = e.Y;

                press_timer = new Timer();
                press_timer.Elapsed += (source, timer_event) =>
                {
                    Dispatcher.Invoke(() =>
                    {
                        press_timer = null;
                        game_player.OnLongPress(x_down, y_down);
                    });

                };
                press_timer.AutoReset = false;
                press_timer.Interval = 500.0;
                press_timer.Start();
            }
        }

        private void GLControl_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (game_player == null) return;
            glControl.MakeCurrent();
            game_player.OnMouseUp(convert_args(e));

            if (press_timer != null)
            {
                press_timer.Stop();
                press_timer = null;
            }
        }

        private void GLControl_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (game_player == null) return;
            glControl.MakeCurrent();
            game_player.OnMouseMove(convert_args(e));

            if (press_timer != null)
            {
                int x = e.X;
                int y = e.Y;

                int dx = x - x_down;
                int dy = y - y_down;
                double dis = Math.Sqrt((double)(dx * dx) + (double)(dy * dy));
                if (dis > 3.0)
                {
                    press_timer.Stop();
                    press_timer = null;
                }

            }
        }

        private void GLControl_MouseWheel(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (game_player == null) return;
            glControl.MakeCurrent();
            game_player.OnMouseWheel(convert_args(e));
        }

        private void GLControl_ControlKey(uint code)
        {
            if (game_player == null) return;
            glControl.MakeCurrent();
            game_player.OnControlKey(code);
        }

        private void GLControl_KeyChar(object sender, System.Windows.Forms.KeyPressEventArgs e)
        {
            if (game_player == null) return;
            glControl.MakeCurrent();
            game_player.OnChar(e.KeyChar);
            e.Handled = true;
        }


        private async Task _doc_save(string filename)
        {
            string text = await GetText();
            File.WriteAllText(filename, text, Encoding.UTF8);
        }

        private void _doc_open(string filename)
        {
            string text = File.ReadAllText(filename, Encoding.UTF8);
            SetText(text);                        
        }

        public async Task<string> doc_save_as()
        {
            var dialog = new SaveFileDialog();
            dialog.Filter = "XML(*.xml)|*.xml";
            if (dialog.ShowDialog() != true)
            {
                return "";
            }
            file_path = dialog.FileName;
            await _doc_save(file_path);
            return file_path;
        }

        public async Task doc_save()
        {
            await _doc_save(file_path);
        }

        public async Task<bool> doc_close()
        {
            bool changed = await TextChanged();
            if (changed)
            {
                var result = MessageBox.Show("File has been modified. Save it?", "Save file", MessageBoxButton.YesNoCancel);
                if (result == MessageBoxResult.Yes)
                {
                    await doc_save();
                    return true;
                }
                else if (result == MessageBoxResult.No)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return true;
            }
        }

        public async Task doc_fresh()
        {
            if (await doc_close())
            {
                _doc_open(file_path);
            }

        }

        private async Task<bool> TextChanged_code()
        {
            string response = await webView.ExecuteScriptAsync("doc_modified()");
            return (bool)JsonConvert.DeserializeObject(response);
        }

        private bool TextChanged_gl()
        {
            string ret = game_player.SendMessageToUser("isModified", "");
            return (bool)JsonConvert.DeserializeObject(ret);
        }

        private async Task<bool> TextChanged()
        {
            if (changed_cache) return true;
            if (cur_tab == 0)
            {
                return await TextChanged_code();
            }
            else
            {
                return TextChanged_gl();
            }
        }

        private async Task<string> GetText_code()
        {
            bool changed = await TextChanged_code();
            if (changed)
            {
                string response = await webView.ExecuteScriptAsync("doc_get_text()");
                text_cache = JsonConvert.DeserializeObject(response).ToString();
                changed_cache = true;
            }
            return text_cache;
        }

        private string GetText_gl()
        {
            bool changed = TextChanged_gl();
            if (changed)
            {
                text_cache = game_player.SendMessageToUser("getXML", "");
                changed_cache = true;
            }
            return text_cache;
        }

        private async Task<string> GetText()
        {
            if (cur_tab == 0)
            {
                await GetText_code();
                SetText_gl(text_cache);
            }
            else
            {
                GetText_gl();
            }
            changed_cache = false;
            return text_cache;
        }

        private async Task SetText_code(string text)
        {
            string para = JsonConvert.SerializeObject(text);
            await webView.ExecuteScriptAsync($"doc_set_text({para})");
        }

        private void SetText_gl(string text)
        {
            _ = game_player.SendMessageToUser("setXML", text);
        }

        private async void SetText(string text)
        {
            text_cache = text;
            changed_cache = false;
            await SetText_code(text);
            SetText_gl(text);
        }


        public void undo()
        {
            if (cur_tab == 0)
            {
                webView.ExecuteScriptAsync("editor.execCommand('undo');");
            }
        }

        public void redo()
        {
            if (cur_tab == 0)
            {
                webView.ExecuteScriptAsync("editor.execCommand('redo');");
            }
        }

        public void comment()
        {
            if (cur_tab == 0)
            {
                webView.ExecuteScriptAsync("editor.execCommand('togglecomment');");
            }
        }

        public void upper()
        {
            if (cur_tab == 0)
            {
                webView.ExecuteScriptAsync("editor.execCommand('touppercase');");
            }
        }

        public void lower()
        {
            if (cur_tab == 0)
            {
                webView.ExecuteScriptAsync("editor.execCommand('tolowercase');");
            }
        }

        public void find()
        {
            if (cur_tab == 0)
            {
                webView.ExecuteScriptAsync("editor.execCommand('find');");
            }
        }

        public void findnext()
        {
            if (cur_tab == 0)
            {
                webView.ExecuteScriptAsync("editor.execCommand('findnext');");
            }
        }

        public void findprev()
        {
            if (cur_tab == 0)
            {
                webView.ExecuteScriptAsync("editor.execCommand('findprevious');");
            }
        }

        public void replace()
        {
            if (cur_tab == 0)
            {
                webView.ExecuteScriptAsync("editor.execCommand('replace');");
            }
        }

        public void gotoline()
        {
            if (cur_tab == 0)
            {
                webView.ExecuteScriptAsync("editor.execCommand('gotoline');");
            }
        }

        private async void tab_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (tab.SelectedIndex == cur_tab) return;

            if (cur_tab == 0)
            {
                if (await TextChanged_code())
                {
                    if (MessageBox.Show("Code has changed, apply it to view?", "Apply changes", MessageBoxButton.YesNo, MessageBoxImage.Warning) == MessageBoxResult.Yes)
                    {
                        await GetText_code();
                        SetText_gl(text_cache);
                    }
                }
            }
            else
            {
                if (tab.SelectedIndex == 0)
                {
                    if (TextChanged_gl())
                    {
                        GetText_gl();
                    }
                    await SetText_code(text_cache);
                }

                if (cur_tab == 1)
                {
                    btn_picking.IsChecked = false;
                }
            }
            
            cur_tab = tab.SelectedIndex;
        }

        private async void btn_apply_Click(object sender, RoutedEventArgs e)
        {
            await GetText_code();
            SetText_gl(text_cache);
        }

        private void btn_picking_Checked(object sender, RoutedEventArgs e)
        {
            scene_graph.IsEnabled = false;
            game_player.SendMessageToUser("picking", "on");
        }

        private void btn_picking_Unchecked(object sender, RoutedEventArgs e)
        {
            game_player.SendMessageToUser("picking", "off");
            scene_graph.IsEnabled = true;
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

        private void update_index_item(TreeViewItem item, JObject obj)
        {
            JObject dict = (JObject)index["index"];
            JArray children = (JArray)obj["children"];
            foreach(string key in children)
            {
                JObject child = (JObject)dict[key];
                var subitem = new TreeViewItem();
                subitem.Tag = key;
                TreeItemMap[key] = subitem;
                string tagName = child["tagName"].ToString();
                JObject attributes = (JObject)child["attributes"];
                string name;
                if (attributes.ContainsKey("name"))
                {
                    name = attributes["name"].ToString();
                }
                else
                {
                    name = tagName;
                }

                string icon_name = "object3d.png";
                if (tagName == "camera")
                {
                    icon_name = "camera.png";
                }
                else if (tagName == "fog")
                {
                    icon_name = "fog.png";
                }
                else if (tagName == "sky")
                {
                    icon_name = "sky.png";
                }
                else if (tagName == "env_light")
                {
                    icon_name = "env_light.png";
                }
                else if (tagName == "control")
                {
                    icon_name = "control.png";
                }
                else if (tagName == "group")
                {
                    icon_name = "group.png";
                }
                else if (tagName == "plane")
                {
                    icon_name = "plane.png";
                }
                else if (tagName == "box")
                {
                    icon_name = "box.png";
                }
                else if (tagName == "sphere")
                {
                    icon_name = "sphere.png";
                }
                else if (tagName == "model")
                {
                    icon_name = "model.png";
                }
                else if (tagName == "avatar")
                {
                    icon_name = "avatar.png";
                }
                else if (tagName == "directional_light")
                {
                    icon_name = "directional_light.png";
                }

                subitem.Header = create_item(name, icon_name);
                subitem.ContextMenu = ctxMenu;
                item.Items.Add(subitem);
                update_index_item(subitem, child);
            }

            item.IsExpanded = true;
        }

        private void update_index()
        {
            scene_graph.Items.Clear();
            TreeItemMap.Clear();

            string key = index["root"].ToString();
            JObject dict = (JObject)index["index"];
            JObject root = (JObject)dict[key];
            var item = new TreeViewItem();
            item.Tag = key;
            TreeItemMap[key] = item;
            string tagName = root["tagName"].ToString();
            JObject attributes = (JObject)root["attributes"];
            string name;
            if (attributes.ContainsKey("name"))
            {
                name = attributes["name"].ToString();
            }
            else
            {
                name = tagName;
            }
            item.Header = create_item(name, "scene.png");
            scene_graph.Items.Add(item);
            update_index_item(item, root);
        }

        private string index_loaded(string json_str)
        {
            index = JObject.Parse(json_str);
            update_index();
            return "";
        }

        private void update_name(object sender, EventArgs e)
        {
            if (picked_key == "") return;
            TreeViewItem item = TreeItemMap[picked_key];
            JObject jobj = (JObject)index["index"][picked_key];
            StackPanel panel = (StackPanel)item.Header;
            TextBlock txt = (TextBlock)panel.Children[1];
            txt.Text = jobj["attributes"]["name"].ToString();
        }

        private HashSet<string> tags3d = new HashSet<string>() { "scene", "group", "plane", "box", "sphere", "model", "avatar", "directional_light" };

        private string picked_key = "";
        private string object_picked(string key)
        {
            picked_key = key;
            property_area.Children.Clear();
            grp_scene_objs.IsEnabled = false;
            grp_3d_objs.IsEnabled = false;
            if (key != "")
            {
                var picked_obj = (JObject)index["index"][key];
                string tag = picked_obj["tagName"].ToString();
                if (tags3d.Contains(tag))
                {
                    grp_3d_objs.IsEnabled = true;
                }
                if (tag == "scene")
                {
                    grp_scene_objs.IsEnabled = true;
                }
                else if (tag == "fog")
                {
                    var tuner = new FogTuner(game_player, picked_obj);
                    property_area.Children.Add(tuner);
                }
                else if (tag == "sky")
                {
                    var tuner = new SkyTuner(game_player, picked_obj);
                    property_area.Children.Add(tuner);
                }
                else if (tag == "env_light")
                {
                    var tuner = new EnvLightTuner(game_player, picked_obj);
                    property_area.Children.Add(tuner);
                }
                else if (tag == "group")
                {
                    var tuner = new GroupTuner(game_player, picked_obj);
                    property_area.Children.Add(tuner);
                    tuner.obj3d_tuner.UpdateName += update_name;
                }
                else if (tag == "plane")
                {
                    var tuner = new PlaneTuner(game_player, picked_obj);
                    property_area.Children.Add(tuner);
                    tuner.obj3d_tuner.UpdateName += update_name;
                }

                var treeItem = TreeItemMap[key];
                treeItem.IsSelected = true;
            }
            btn_picking.IsChecked = false;
            return "";
        }

        private void scene_graph_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
        {
            if (btn_picking.IsChecked == true) return;
            var item = (TreeViewItem)scene_graph.SelectedItem;
            if (item!=null)
            {
                string key = (string)item.Tag;
                game_player.SendMessageToUser("pick_obj", key);
            }
        }

        private string object_created(string json_str)
        {
            JObject info = JObject.Parse(json_str);
            foreach (var item in info)
            {
                string key = item.Key;
                JObject node = (JObject)item.Value;
                string key_parent = node["parent"].ToString();
                JObject parent = (JObject)index["index"][key_parent];
                index["index"][key] = node;
                JArray children = (JArray)parent["children"];
                children.Add(key);
            }
            update_index();
            return "";
        }

        private string object_removed(string key)
        {
            JObject node = (JObject)index["index"][key];            
            ((JObject)index["index"]).Remove(key);

            string key_parent = node["parent"].ToString();
            JObject parent = (JObject)index["index"][key_parent];
            JArray children = (JArray)parent["children"];

            foreach (var child_key in children)
            {
                if ((string)child_key == key)
                {
                    children.Remove(child_key);
                    break;
                }
            }            

            update_index();
            return "";
        }

        private void req_create_scene_obj(string tag)
        {           
            JObject base_obj = (JObject)index["index"][picked_key];
            JArray children = (JArray)base_obj["children"];

            string key_existing = "";           
            foreach (string key in base_obj["children"])
            {
                JObject child = (JObject)index["index"][key];
                if (child["tagName"].ToString() == tag)
                {
                    key_existing = key;
                    break;
                }
            }

            if (key_existing!="")
            {
                game_player.SendMessageToUser("pick_obj", key_existing);
                return;
            }            

            JObject req = new JObject();
            req["base_key"] = picked_key;
            req["tag"] = tag;
            game_player.SendMessageToUser("create", req.ToString());
        }

        private void req_create_obj3d(string tag)
        {            
            JObject req = new JObject();
            req["base_key"] = picked_key;
            req["tag"] = tag;
            game_player.SendMessageToUser("create", req.ToString());
        }

        private void btn_create_fog_Click(object sender, RoutedEventArgs e)
        {
            req_create_scene_obj("fog");
        }

        private void btn_create_sky_Click(object sender, RoutedEventArgs e)
        {
            req_create_scene_obj("sky");
        }

        private void btn_create_env_light_Click(object sender, RoutedEventArgs e)
        {
            req_create_scene_obj("env_light");
        }

        private void btn_create_group_Click(object sender, RoutedEventArgs e)
        {
            req_create_obj3d("group");
        }

        private void btn_create_plane_Click(object sender, RoutedEventArgs e)
        {
            req_create_obj3d("plane");
        }

        private void remove_Click(object sender, RoutedEventArgs e)
        {
            MenuItem mi = sender as MenuItem;
            if (mi != null)
            {
                ContextMenu cm = (ContextMenu)mi.Parent;
                TreeViewItem item = (TreeViewItem)cm.PlacementTarget;                
                string key = (string)item.Tag;
                JObject obj = (JObject)index["index"][key];
                string tag = (string)obj["tagName"];
                JObject att = (JObject)obj["attributes"];
                string name;
                if (att.ContainsKey("name"))
                {
                    name = (string)att["name"];
                }
                else
                {
                    name = tag;
                }

                if (MessageBox.Show($"Remove {tag} object \"{name}\" and all its children from scene?", "Remove Object", MessageBoxButton.YesNo, MessageBoxImage.Warning) == MessageBoxResult.Yes)
                {
                    game_player.SendMessageToUser("remove", key);
                }
            }
            
        }
    }
}
