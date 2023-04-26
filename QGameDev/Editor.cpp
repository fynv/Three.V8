#include "Editor.h"

EditorBase::EditorBase(QWidget* parent) : QWidget(parent)
{

}
EditorBase::~EditorBase()
{

}

void EditorBase::cleanup()
{

}

void EditorBase::doc_close()
{
	emit doc_close_ret(true);
}

Editor::Editor(QWidget* parent): EditorBase(parent)
{

}

Editor::~Editor()
{

}