#include <string>

class EditorActionBase
{
public:
	virtual void Execute(){};
	virtual void Undo() = 0;
	virtual void Redo() = 0;

	virtual std::string ActionName() = 0;

	virtual ~EditorActionBase(){};
};