#include "HardwareManager.h"
#include <Taskschd.h>

#pragma comment(lib, "Taskschd.lib")

HardwareManagerNamespaceBegin

CTaskScheduler::CTaskScheduler() :
	m_pTaskDef(NULL),
	m_pRootTaskFolder(NULL)
{
	ITaskService *pTaskService = NULL;
	HRESULT hr = ::CoCreateInstance( 
		CLSID_TaskScheduler,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_ITaskService,
		(void**)&pTaskService);
	if (FAILED(hr)) return;
	hr = pTaskService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
	if (FAILED(hr)) {
		pTaskService->Release();
		return;
	}
	// ��ȡ���ļ���
	hr = pTaskService->GetFolder( _bstr_t( L"\\"), &m_pRootTaskFolder);
	if (FAILED(hr)) {
		pTaskService->Release();
		return;
	}
	// ��������
	hr = pTaskService->NewTask(0, &m_pTaskDef);
	if (FAILED(hr)) {
		m_pRootTaskFolder->Release();
		pTaskService->Release();
		return;
	}
}
CTaskScheduler::~CTaskScheduler()
{
	if (m_pTaskDef) {
		m_pTaskDef->Release();
		m_pTaskDef = NULL;
	}
	if (m_pRootTaskFolder) {
		m_pRootTaskFolder->Release();
		m_pRootTaskFolder = NULL;
	}
}

bool CTaskScheduler::Delete(const wchar_t *FolderName, const wchar_t *TaskName)
{
	if (!TaskName) return false;
	ITaskService* pTaskService = NULL;
	ITaskFolder* pRootFolder = NULL;
	ITaskFolder* pMyFolder = NULL;
	HRESULT hr = ::CoCreateInstance(
		CLSID_TaskScheduler,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_ITaskService,
		(void**)&pTaskService);
	if (FAILED(hr)) return false;
	hr = pTaskService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
	if (FAILED(hr)) {
		pTaskService->Release();
		return false;
	}
	// ��ȡ���ļ���
	hr = pTaskService->GetFolder( _bstr_t( L"\\"), &pRootFolder);
	if (FAILED(hr)) {
		pTaskService->Release();
		return false;
	}
	if (FolderName) {
		hr = pRootFolder->GetFolder(_bstr_t(FolderName), &pMyFolder);
		if (FAILED(hr)) {
			pRootFolder->Release();
			pTaskService->Release();
			return false;
		}
	}
	if (pMyFolder) hr = pMyFolder->DeleteTask(_bstr_t(TaskName), 0);
	else hr = pRootFolder->DeleteTask(_bstr_t(TaskName), 0);
	if (FAILED(hr)) {
		if (pMyFolder) pMyFolder->Release();
		pRootFolder->Release();
		pTaskService->Release();
		return false;
	}
	if (pMyFolder) pMyFolder->Release();
	pRootFolder->Release();
	pTaskService->Release();
	return true;
}

bool CTaskScheduler::Action(const wchar_t *ExePath, const wchar_t *Param, const wchar_t *WorkingDir)
{
	if (!ExePath) return false;
	if (!m_pTaskDef) return false;
	IActionCollection* pActionCollection = NULL;
	IAction* pAction = NULL;
	IExecAction* pExecAction = NULL;
	//�õ���������    
	HRESULT hr = m_pTaskDef->get_Actions(&pActionCollection);
	if (FAILED(hr)) return false;
	//�ڶ��������д�������    
	hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
	if (FAILED(hr)) {
		pActionCollection->Release();
		return false;
	}
	//��������д��ִ�г���
	hr = pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction);
	if (FAILED(hr)) {
		pAction->Release();
		pActionCollection->Release();
		return false;
	}
	// ���б�����
	hr = pExecAction->put_Path(_bstr_t(ExePath));
	if (NULL != Param) {
		hr = pExecAction->put_Arguments(_bstr_t(Param));
		if (FAILED(hr)) {
			pExecAction->Release();
			pAction->Release();
			pActionCollection->Release();
			return false;
		}
	}
	if (NULL != WorkingDir) {
		hr = pExecAction->put_WorkingDirectory(_bstr_t(WorkingDir));
		if (FAILED(hr)) {
			pExecAction->Release();
			pAction->Release();
			pActionCollection->Release();
			return false;
		}
	}
	pExecAction->Release();
	pAction->Release();
	pActionCollection->Release();
	return true;
}
bool CTaskScheduler::Trigger(unsigned int Trigger,const wchar_t *Time)
{
	if (Trigger != 1 && Trigger != 2 && Trigger != 3) return false;
	if (Trigger == 3 && Time == NULL) return false;
	if (!m_pTaskDef) return false;
	ITriggerCollection *pTriggerCollection = NULL;
	ITrigger *pTrigger = NULL;
	//�õ�����������    
	HRESULT hr = m_pTaskDef->get_Triggers(&pTriggerCollection);
	if (FAILED(hr)) return false;
	//�ڴ����������д���������
	if (1 == Trigger) hr = pTriggerCollection->Create(TASK_TRIGGER_BOOT, &pTrigger);// ϵͳ����ʱ����
	else if (2 == Trigger) hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);// ���û���¼ʱ����
	else if (3 == Trigger) {
		hr = pTriggerCollection->Create(TASK_TRIGGER_TIME, &pTrigger);
		hr = pTrigger->put_StartBoundary(_bstr_t(Time));
	}
	if (FAILED(hr)) {
		pTriggerCollection->Release();
		if (pTrigger) pTrigger->Release();
		return false;
	}
	pTriggerCollection->Release();
	if (pTrigger) pTrigger->Release();
	return true;
}
bool CTaskScheduler::Register(const wchar_t *FolderName, const wchar_t *TaskName)
{
	if (!TaskName) return false;
	if (!m_pTaskDef || !m_pRootTaskFolder) return false;
	HRESULT hr = S_OK;
	ITaskFolder *pNewFolder = NULL;
	IRegisteredTask *pRegisteredTask = NULL;
	ITaskFolder *pMyFolder = NULL;
	if (FolderName) {
		hr = m_pRootTaskFolder->GetFolder(_bstr_t(FolderName), &pNewFolder);
		// ����ļ��в�����,�ʹ���һ��
		if (FAILED(hr)) {
			hr = m_pRootTaskFolder->CreateFolder(_bstr_t(FolderName), _variant_t(), &pNewFolder);
			if (FAILED(hr)) {
				m_pRootTaskFolder->Release();
				return false;
			}
		}
	}
	// ���ָ�����ļ���, ����ָ�����ļ����´�������ƻ�, �����ڸ�Ŀ¼�´�������ƻ�
	if (pNewFolder) pMyFolder = pNewFolder;
	else pMyFolder = m_pRootTaskFolder;
	hr = pMyFolder->RegisterTaskDefinition(
		_bstr_t(TaskName),
		m_pTaskDef,
		TASK_CREATE_OR_UPDATE,  
		_variant_t(),
		_variant_t(),
		TASK_LOGON_INTERACTIVE_TOKEN,
		_variant_t(L""),
		&pRegisteredTask);
	if (FAILED(hr)) {
		if (pNewFolder) pNewFolder->Release();
		m_pRootTaskFolder->Release();
		return false;
	}
	if (pNewFolder) pNewFolder->Release();
	m_pRootTaskFolder->Release();
	return true;
}
bool CTaskScheduler::Principal(bool HighestLevel)
{
	if (!m_pTaskDef) return false;
	IPrincipal *pPrincipal = NULL;
	HRESULT hr = m_pTaskDef->get_Principal(&pPrincipal);
	if (FAILED(hr)) return false;
	hr = pPrincipal->put_RunLevel(HighestLevel ? TASK_RUNLEVEL_HIGHEST : TASK_RUNLEVEL_LUA);
	if (FAILED(hr)) {
		pPrincipal->Release();
		return false;
	}
	pPrincipal->Release();
	return true;
}
bool CTaskScheduler::SetStartOnBattery(bool StartOnBattery)
{
	if (!m_pTaskDef) return false;
	ITaskSettings *pSettings = NULL;
	HRESULT hr = m_pTaskDef->get_Settings(&pSettings);
	if (FAILED(hr)) return false;
	hr = pSettings->put_DisallowStartIfOnBatteries(VARIANT_BOOL(!StartOnBattery));
	if (FAILED(hr)) {
		pSettings->Release();
		return false;
	}
	pSettings->Release();
	return true;
}
bool CTaskScheduler::SetWakeToRun(bool WakeToRun)
{
	if (!m_pTaskDef) return false;
	ITaskSettings *pSettings = NULL;
	HRESULT hr = m_pTaskDef->get_Settings(&pSettings);
	if (FAILED(hr)) return false;
	hr = pSettings->put_WakeToRun(VARIANT_BOOL(!WakeToRun));
	if (FAILED(hr)) {
		pSettings->Release();
		return false;
	}
	pSettings->Release();
	return true;
}

HardwareManagerNamespaceEnd
