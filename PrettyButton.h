#pragma once

class CPrettyButton : public CButton
{
public:
	CPrettyButton();
	virtual ~CPrettyButton();
private:
	bool forceHighlight;
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	virtual void PreSubclassWindow();
public:
	void SetForceHighlight(bool force)
	{ 
		if(force!=this->forceHighlight)
		{
			this->forceHighlight = force;
			Invalidate();
		}
		this->forceHighlight = force;
	}
	bool GetForceHighligt() { return forceHighlight; }
};