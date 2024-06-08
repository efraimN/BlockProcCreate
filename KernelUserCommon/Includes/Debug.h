#pragma once

class DebugClass
{
public:
	DebugClass(void(*Funct)(BOOL))
	{
		m_Funct = Funct;
		m_Funct(TRUE);
	};
	virtual ~DebugClass()
	{
		m_Funct(FALSE);
	};

public:
	void(*m_Funct)(BOOL);
protected:
private:
};


#define PROC_ENTRY 	DebugClass m((void(*)(BOOL))ProcEntry)

