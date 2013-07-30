#include "stdafx.h"
#include "Buffer.h"
#include "math.h"

CBuffer::CBuffer()
{
	m_nSize = 0;
	m_pPtr = m_pBase = NULL;

	InitializeCriticalSection(&m_cs);
}

CBuffer::~CBuffer()
{
	if (m_pBase)
	{
		VirtualFree(m_pBase, 0, MEM_RELEASE);
	}

	DeleteCriticalSection(&m_cs);
}

//从尾插入
BOOL CBuffer::Write(PBYTE pData, UINT nSize)
{
	EnterCriticalSection(&m_cs);

	if (-1 == ReAllocateBuffer(nSize + GetBufferLen()))
	{
		LeaveCriticalSection(&m_cs);
		return FALSE;
	}

	CopyMemory(m_pPtr, pData, nSize);

	m_pPtr += nSize;

	LeaveCriticalSection(&m_cs);
	return nSize;
}

//从头开始插入
BOOL CBuffer::Insert(PBYTE pData, UINT nSize)
{
	EnterCriticalSection(&m_cs);

	if (-1 == ReAllocateBuffer(nSize + GetBufferLen()))
	{
		LeaveCriticalSection(&m_cs);
		return FALSE;
	}

	MoveMemory(m_pBase + nSize, m_pBase, GetMemSize() - nSize);
	CopyMemory(m_pBase, pData, nSize);

	m_pPtr += nSize;
	
	LeaveCriticalSection(&m_cs);
	return nSize;
}

//从缓冲区开始拿出数据，剩下的数据前移
UINT CBuffer::Read(PBYTE pData, UINT nSize)
{
	EnterCriticalSection(&m_cs);

	if (nSize > GetMemSize())
	{
		LeaveCriticalSection(&m_cs);
		return 0;
	}

	if (nSize > GetBufferLen())
	{
		nSize = GetBufferLen();
	}

	if (nSize)
	{
		CopyMemory(pData, m_pBase, nSize);
		MoveMemory(m_pBase, m_pBase + nSize, GetMemSize() - nSize);

		m_pPtr -= nSize;
	}
		
	DeAllocateBuffer(GetBufferLen());

	LeaveCriticalSection(&m_cs);
	return nSize;
}

UINT CBuffer::GetMemSize() 
{
	return m_nSize;
}

UINT CBuffer::GetBufferLen() 
{
	if (m_pBase == NULL)
		return 0;

	return m_pPtr - m_pBase;
}

UINT CBuffer::ReAllocateBuffer(UINT nRequestedSize)
{
	if (nRequestedSize < GetMemSize())
		return 0;

	UINT nNewSize = (UINT)ceil(nRequestedSize / 1024.0) * 1024;

	PBYTE pNewBuffer = (PBYTE)VirtualAlloc(NULL, nNewSize, MEM_COMMIT, PAGE_READWRITE);
	if (pNewBuffer == NULL)
		return -1;

	UINT nBufferLen = GetBufferLen();
	CopyMemory(pNewBuffer, m_pBase, nBufferLen);

	if (m_pBase)
	{
		VirtualFree(m_pBase, 0, MEM_RELEASE);
	}

	m_pBase = pNewBuffer;
	m_pPtr = m_pBase + nBufferLen;
	m_nSize = nNewSize;

	return m_nSize;
}

UINT CBuffer::DeAllocateBuffer(UINT nRequestedSize)
{
	if (nRequestedSize < GetBufferLen())
	{
		return 0;
	}

	UINT nNewSize = (UINT) ceil(nRequestedSize / 1024.0) * 1024;

	if (nNewSize < GetMemSize())
	{
		return 0;
	}

	PBYTE pNewBuffer = (PBYTE)VirtualAlloc(NULL, nNewSize, MEM_COMMIT, PAGE_READWRITE);

	UINT nBufferLen = GetBufferLen();
	CopyMemory(pNewBuffer, m_pBase, nBufferLen);

	VirtualFree(m_pBase, 0, MEM_RELEASE);

	m_pBase = pNewBuffer;
	m_pPtr = m_pBase + nBufferLen;
	m_nSize = nNewSize;

	return m_nSize;
}

int CBuffer::Scan(PBYTE pScan, UINT nPos)
{
	if (nPos > GetBufferLen())
	{
		return -1;
	}

	PBYTE pMatch = NULL;
	pMatch = (PBYTE)strstr((char*)(m_pBase + nPos),(char*)pScan);
	if (NULL == pMatch)
		return -1;

	int nOffset = 0;
	if (pMatch)
		nOffset = (pMatch - m_pBase) + strlen((char*)pScan);

	return nOffset;
}

void CBuffer::ClearBuffer()
{
	EnterCriticalSection(&m_cs);

	m_pPtr = m_pBase;

	DeAllocateBuffer(1024);
	LeaveCriticalSection(&m_cs);
}

void CBuffer::Copy(CBuffer& buffer)
{
	int nReSize = buffer.GetMemSize();
	int nSize = buffer.GetBufferLen();
	ClearBuffer();

	if (-1 == ReAllocateBuffer(nReSize))
	{
		return;
	}

	m_pPtr = m_pBase + nSize;

	CopyMemory(m_pBase, buffer.GetBuffer(), buffer.GetBufferLen());
}

PBYTE CBuffer::GetBuffer(UINT nPos)
{
	return m_pBase + nPos;
}

UINT CBuffer::Delete(UINT nSize)
{
	if (nSize > GetMemSize())
	{
		return 0;
	}

	if (nSize > GetBufferLen())
	{
		nSize = GetBufferLen();
	}

	if (nSize)
	{
		MoveMemory(m_pBase, m_pBase + nSize, GetMemSize() - nSize);
		m_pPtr -= nSize;
	}
		
	DeAllocateBuffer(GetBufferLen());
	return nSize;
}