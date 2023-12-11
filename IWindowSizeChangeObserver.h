#pragma once
class IWindowSizeChangeObserver
{
public:
    virtual ~IWindowSizeChangeObserver() = default;

    //Called when the window size changes.
    virtual void onWindowSizeChanged(int width, int height) = 0;
};