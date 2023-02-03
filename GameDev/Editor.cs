using System;
using System.Threading.Tasks;

namespace GameDev
{
    internal interface EditorBase
    {
        void cleanup();
        Task<bool> doc_close();
    }

    internal interface Editor : EditorBase
    {
        Task<string> doc_save_as();
        Task doc_save();                
        Task doc_fresh();

        void undo();
        void redo();
        void comment();
        void upper();
        void lower();
        void find();
        void findnext();
        void findprev();
        void replace();
        void gotoline();

    }
}
