#pragma once

struct ITaskDefinition;
struct ITaskFolder;

HardwareManagerNamespaceBegin

class CTaskScheduler
{
public:
	CTaskScheduler();
	virtual ~CTaskScheduler();

private:
	ITaskDefinition *m_pTaskDef; // 任务计划定义对象
    ITaskFolder *m_pRootTaskFolder; // 任务计划根文件夹对象

public:
	static bool Delete(const wchar_t *FolderName, const wchar_t *TaskName);

public:
	bool Action(const wchar_t *ExePath, const wchar_t *Param, const wchar_t *WorkingDir);
	bool Trigger(unsigned int Trigger,const wchar_t *Time);
	bool Register(const wchar_t *FolderName, const wchar_t *TaskName);
	bool Principal(bool HighestLevel);// 可能需要管理员权限
	bool SetStartOnBattery(bool StartOnBattery);
	bool SetWakeToRun(bool WakeToRun);
};

HardwareManagerNamespaceEnd
