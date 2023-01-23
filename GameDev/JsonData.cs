using System;
using System.IO;
using Newtonsoft.Json.Linq;

namespace GameDev
{
    public class JsonData
    {
        public String filename = "";
        public JToken data = null;        

        public void Clear()
        {
            filename = "";
            data = null;
        }

        public void Load()
        {
            data = JToken.Parse(File.ReadAllText(filename));
        }

        public void Save()
        {
            File.WriteAllText(filename, data.ToString());
        }

    }
}
