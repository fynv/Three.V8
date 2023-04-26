#pragma once

#include <QWidget>

class EditorBase : public QWidget
{
	Q_OBJECT
public:
	EditorBase(QWidget* parent);
	~EditorBase();

	virtual void cleanup();
	virtual void doc_close(); // async

signals:	
	void doc_close_ret(bool res);

};

class Editor : public EditorBase
{
	Q_OBJECT
public:
	Editor(QWidget* parent);
	~Editor();

	virtual void doc_save_as() = 0; // async	
	virtual void doc_save() = 0;// async	
	virtual void doc_refresh() = 0; //async

	virtual void undo() {}
	virtual void redo() {}
	virtual void comment() {}
	virtual void upper() {}
	virtual void lower() {}
	virtual void find() {}
	virtual void findnext() {}
	virtual void findprev() {}
	virtual void replace() {}
	virtual void gotoline() {}

signals:
	void doc_save_as_ret(QString filename);
	void doc_save_ret();
	void doc_refresh_ret();

};
