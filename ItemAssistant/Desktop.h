#ifndef DESKTOP_H
#define DESKTOP_H

namespace aoia
{
    class Desktop
    {
    public:
        Desktop(void);
        virtual ~Desktop(void);

        bool ContainsPoint(int x, int y) const;
        bool IntersectsRect(RECT rect) const;
    };
}

#endif // DESKTOP_H
