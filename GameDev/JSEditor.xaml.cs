using System;
using System.Text;
using System.IO;
using System.Diagnostics;
using System.Windows.Controls;
using System.Threading.Tasks;
using Newtonsoft.Json;
using System.Windows;
using Microsoft.Win32;

namespace GameDev
{
    /// <summary>
    /// Interaction logic for JSEditor.xaml
    /// </summary>
    public partial class JSEditor : UserControl, Editor
    {
        private string file_path;
        public JSEditor(string file_path)
        {
            InitializeComponent();
            this.file_path = file_path;

            string local_path = Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName);
            webView.Source = new Uri($"{local_path}/editor/js_editor.html");
            webView.NavigationCompleted += (sender, e) =>
            {
                _doc_open(file_path);
            };
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
            dialog.Filter = "Javascript|*.js";
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
                    doc_save();
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


        private async Task<bool> TextChanged()
        {
            string response = await webView.ExecuteScriptAsync("doc_modified()");
            return (bool)JsonConvert.DeserializeObject(response);
        }

        private async Task<string> GetText()
        {
            string response = await webView.ExecuteScriptAsync("doc_get_text()");
            return JsonConvert.DeserializeObject(response).ToString();
        }

        private async void SetText(string text)
        {
            string para = JsonConvert.SerializeObject(text);
            await webView.ExecuteScriptAsync($"doc_set_text({para})");
        }


        public void undo()
        {
            webView.ExecuteScriptAsync("editor.execCommand('undo');");
        }

        public void redo()
        {
            webView.ExecuteScriptAsync("editor.execCommand('redo');");
        }

        public void comment()
        {
            webView.ExecuteScriptAsync("editor.execCommand('togglecomment');");
        }

        public void upper()
        {
            webView.ExecuteScriptAsync("editor.execCommand('touppercase');");
        }

        public void lower()
        {
            webView.ExecuteScriptAsync("editor.execCommand('tolowercase');");
        }

        public void find()
        {
            webView.ExecuteScriptAsync("editor.execCommand('find');");
        }

        public void findnext()
        {
            webView.ExecuteScriptAsync("editor.execCommand('findnext');");
        }

        public void findprev()
        {
            webView.ExecuteScriptAsync("editor.execCommand('findprevious');");
        }

        public void replace()
        {
            webView.ExecuteScriptAsync("editor.execCommand('replace');");
        }

        public void gotoline()
        {
            webView.ExecuteScriptAsync("editor.execCommand('gotoline');");
        }
    }
}
