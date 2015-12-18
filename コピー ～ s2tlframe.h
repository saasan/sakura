#ifndef __S2TLFRAME_H__
#define __S2TLFRAME_H__

#pragma once

/////////////////////////////////////////////////////////////////////////////
// Classes in this file
//
// CSnapWindow<T>


namespace WTL
{

/////////////////////////////////////////////////////////////////////////////
// CSnapWindow - provides support for snapping windows

template <class T>
class CSnapWindow
{
public:
// Data declarations and members
	BOOL m_bEnable;				// �X�i�b�v�̗L��/����
	DWORD m_lSnapWidth;			// �X�i�b�v���镝
	POINT m_ptNCLButtonDown;

// Constructor
	CSnapWindow(BOOL bEnable = FALSE, DWORD lSnapWidth = 10)
	{
		m_bEnable = bEnable;
		m_lSnapWidth = lSnapWidth;
		::ZeroMemory(&m_ptNCLButtonDown, sizeof(m_ptNCLButtonDown));
	}

// Operations
	VOID SetSnapEnable(const BOOL bEnable)
	{
		m_bEnable = bEnable;
	}

	BOOL GetSnapEnable() const
	{
		return m_bEnable;
	}

	VOID SetSnapWidth(const DWORD lWidth)
	{
		m_lSnapWidth = lWidth;
	}

	LONG GetSnapWidth() const
	{
		return m_lSnapWidth;
	}

// Message map and handlers
	BEGIN_MSG_MAP(CSnapWindow)
		MESSAGE_HANDLER(WM_MOVING, OnMoving)
		MESSAGE_HANDLER(WM_NCLBUTTONDOWN, OnNcLButtonDown)
	END_MSG_MAP()

	LRESULT OnMoving(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		RECT rcDesktop;
		POINT ptCurPos;

		if (!m_bEnable || !::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0) || !::GetCursorPos(&ptCurPos))
		{
			bHandled = FALSE;
			return FALSE;
		}

		LONG lSnapWidth = m_lSnapWidth;
		LPRECT lprc = (LPRECT)lParam;

		POINT ptWindowSize = {
			(lprc->right - lprc->left),								// �E�C���h�E�̕�
			(lprc->bottom - lprc->top)								// �E�C���h�E�̍���
		};

		RECT rcWindow = {
			(ptCurPos.x - m_ptNCLButtonDown.x),							// �J�[�\���ʒu�ɑ΂���E�C���h�E�̍���
			(ptCurPos.y - m_ptNCLButtonDown.y),							// �J�[�\���ʒu�ɑ΂���E�C���h�E�̏㑤
			(ptCurPos.x + (ptWindowSize.x - m_ptNCLButtonDown.x)),		// �J�[�\���ʒu�ɑ΂���E�C���h�E�̉E��
			(ptCurPos.y + (ptWindowSize.y - m_ptNCLButtonDown.y))		// �J�[�\���ʒu�ɑ΂���E�C���h�E�̉���
		};

		// ���E
		if (((rcDesktop.left - lSnapWidth) < rcWindow.left) && (rcWindow.left < (rcDesktop.left + lSnapWidth)))
		{
			// �f�X�N�g�b�v�̍����ɋ߂Â����ꍇ�A���ɂ�������
			lprc->left = rcDesktop.left;
		}
		else if (((rcDesktop.right - lSnapWidth) < rcWindow.right) && (rcWindow.right < (rcDesktop.right + lSnapWidth)))
		{
			// �f�X�N�g�b�v�̉E���ɋ߂Â����ꍇ�A�E�ɂ�������
			lprc->left = rcDesktop.right - ptWindowSize.x;
		}
		else
		{
			lprc->left = ptCurPos.x - m_ptNCLButtonDown.x;
		}
		lprc->right = lprc->left + ptWindowSize.x;

		// �㉺
		if (((rcDesktop.top - lSnapWidth) < rcWindow.top) && (rcWindow.top < (rcDesktop.top + lSnapWidth)))
		{
			// �f�X�N�g�b�v�̏㑤�ɋ߂Â����ꍇ�A��ɂ�������
			lprc->top = rcDesktop.top;
		}
		else if (((rcDesktop.bottom - lSnapWidth) < rcWindow.bottom) && (rcWindow.bottom < (rcDesktop.bottom + lSnapWidth)))
		{
			// �f�X�N�g�b�v�̉����ɋ߂Â����ꍇ�A���ɂ�������
			lprc->top = rcDesktop.bottom - ptWindowSize.y;
		}
		else
		{
			lprc->top = ptCurPos.y - m_ptNCLButtonDown.y;
		}
		lprc->bottom = lprc->top + ptWindowSize.y;

		return TRUE;
	}

	LRESULT OnNcLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_bEnable)
		{
			T* pT = static_cast<T*>(this);
			INT nHittest = (INT)wParam;		// hit-test value

			if (nHittest == HTCAPTION)
			{
				POINTS pts = MAKEPOINTS(lParam);	// position of cursor
				RECT rc;
				
				if (pT->GetWindowRect(&rc))
				{
					m_ptNCLButtonDown.x = pts.x - rc.left;
					m_ptNCLButtonDown.y = pts.y - rc.top;
				}

			}
		}

		bHandled = FALSE;
		return FALSE;
	}
};

}; //namespace WTL

#endif // __S2TLFRAME_H__
