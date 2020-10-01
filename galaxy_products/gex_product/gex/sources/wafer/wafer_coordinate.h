#ifndef WAFER_COORDINATE_H
#define WAFER_COORDINATE_H

namespace GS
{
namespace Gex
{

class WaferCoordinate
{

public:

    /*!
      @brief    Constructs a invalid wafer coordinate
      */
    WaferCoordinate();

    /*!
      @brief    Constructs a wafer coordinate with the given x and y

      @sa       SetX, SetY
      */
    WaferCoordinate(int x, int y);

    /*!
      @brief    Copy constructor
      */
    WaferCoordinate(const WaferCoordinate& other);

    /*!
      @brief    Destructor
      */
    ~WaferCoordinate();

    /*!
      @brief    Returns whether the coordinate is valid.

      @return   True if the coordinate is valid

      @sa       IsValidX, IsValidY
      */
    bool    IsValid() const;

    /*!
      @brief    Returns whether the X coordinate is valid.

      @return   True if the X coordinate is valid

      @sa       IsValidY, IsValid
      */
    bool    IsValidX() const;

    /*!
      @brief    Returns whether the Y coordinate is valid.

      @return   True if the Y coordinate is valid

      @sa       IsValidX, IsValid
      */
    bool    IsValidY() const;

    /*!
      @brief    Returns the X coordinate of the die

      @return   The X coordinate.

      @sa       SetX, GetY
      */
    int     GetX() const;

    /*!
      @brief    Returns the Y coordinate

      @return   The Y coordinate.

      @sa       SetY, GetX
      */
    int     GetY() const;

    /*!
      @brief    Set the X coordinate

      @param    x   Position on the X axis

      @sa       GetX, SetY
      */
    void    SetX(int x);

    /*!
      @brief    Set the y coordinate

      @param    y   Position on the X axis

      @sa       GetY, SetX
      */
    void    SetY(int y);

    /*!
      @brief    Moves the wafer coordinate x along the X axis and y along the Y axis,
                relative to the current position.

      @param    x   Offset to apply on X axis
      @param    y   Offset to apply on Y axis
      */
    void    Translate(int x, int y);

    /*!
      @brief    Returns a copy of the coordinate translated x along the X axis and y along the Y axis,
                relative to the current position.

      @param    x   Offset to apply on X axis
      @param    y   Offset to apply on Y axis
      */
    WaferCoordinate     Translated(int x, int y) const;

    /*!
      @brief    Assignement operator
      */
    WaferCoordinate&    operator=(const WaferCoordinate& other);

    /*!
      @brief    Returns true if this coordinate is equal to the given coordinate
      */
    bool                operator==(const WaferCoordinate& other) const;

    /*!
      @brief    Returns true if this coordinate is not equal to the given coordinate
      */
    bool                operator!=(const WaferCoordinate& other) const;

    /*!
      @brief    Substracts the given coordinate to this coordinate and returns a reference to this coordinate.

      @sa       operator-=
      */
    WaferCoordinate&    operator-=(const WaferCoordinate& other);

    /*!
      @brief    Adds the given coordinate to this coordinate and returns a reference to this coordinate.

      @sa       operator+=
      */
    WaferCoordinate&    operator+=(const WaferCoordinate& other);

    /*!
      @brief    Returns true if this coordinate is lesser than the given coordinate

      @sa       operator<
      */
    bool                operator<(const WaferCoordinate& other) const;

    /*!
      @brief    Substracts the given coordinate to this coordinate and returns a new coordinate.

      @sa       operator-
      */
    WaferCoordinate     operator-(const WaferCoordinate& other);

    /*!
      @brief    Adds the given coordinate to this coordinate and returns a new coordinate.

      @sa       operator+
      */
    WaferCoordinate    operator+(const WaferCoordinate& other);

private:

    int     mX;
    int     mY;
};

} // end namespace Gex
} // end namespace GS

#endif // WAFER_COORDINATE_H
