#pragma once

class nonCopyable
{
private:
	nonCopyable(const nonCopyable&) = delete;
	nonCopyable& operator =(const nonCopyable&) = delete;
protected:
	nonCopyable() = default;
	nonCopyable(nonCopyable&) = default;

    nonCopyable(nonCopyable&&) noexcept = default;
    nonCopyable& operator=(nonCopyable&&) noexcept = default;
};


