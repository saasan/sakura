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
	BOOL m_bEnable;				// スナップの有効/無効
	DWORD m_lSnapWidth;			// スナップする幅
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
			(lprc->right - lprc->left),								// ウインドウの幅
			(lprc->bottom - lprc->top)								// ウインドウの高さ
		};

		RECT rcWindow = {
			(ptCurPos.x - m_ptNCLButtonDown.x),							// カーソル位置に対するウインドウの左側
			(ptCurPos.y - m_ptNCLButtonDown.y),							// カーソル位置に対するウインドウの上側
			(ptCurPos.x + (ptWindowSize.x - m_ptNCLButtonDown.x)),		// カーソル位置に対するウインドウの右側
			(ptCurPos.y + (ptWindowSize.y - m_ptNCLButtonDown.y))		// カーソル位置に対するウインドウの下側
		};

		// 左右
		if (((rcDesktop.left - lSnapWidth) < rcWindow.left) && (rcWindow.left < (rcDesktop.left + lSnapWidth)))
		{
			// デスクトップの左側に近づいた場合、左にくっつける
			lprc->left = rcDesktop.left;
		}
		else if (((rcDesktop.right - lSnapWidth) < rcWindow.right) && (rcWindow.right < (rcDesktop.right + lSnapWidth)))
		{
			// デスクトップの右側に近づいた場合、右にくっつける
			lprc->left = rcDesktop.right - ptWindowSize.x;
		}
		else
		{
			lprc->left = ptCurPos.x - m_ptNCLButtonDown.x;
		}
		lprc->right = lprc->left + ptWindowSize.x;

		// 上下
		if (((rcDesktop.top - lSnapWidth) < rcWindow.top) && (rcWindow.top < (rcDesktop.top + lSnapWidth)))
		{
			// デスクトップの上側に近づいた場合、上にくっつける
			lprc->top = rcDesktop.top;
		}
		else if (((rcDesktop.bottom - lSnapWidth) < rcWindow.bottom) && (rcWindow.bottom < (rcDesktop.bottom + lSnapWidth)))
		{
			// デスクトップの下側に近づいた場合、下にくっつける
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
