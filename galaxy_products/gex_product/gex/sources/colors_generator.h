#ifndef COLORS_GENERATOR_H
#define COLORS_GENERATOR_H

#include <QVector>
#include <QColor>

//____________________________________________________________________
// Static class for generating colors table with colors shades
// Version 29 july 2009 : 17h26
// 1 : set the dims with ColorsGenerator::setDim() (the colors are generated here)
// 2 : get your colors with ColorsGenerator::getColor()
// No need to instantiate, nor to delete. Ready to use.

class ColorsGenerator : public QObject
{
    Q_OBJECT
 public:
    // First, set the dimemsions of the table : number of colors and number of shades per colors.
    // WARNING : if the number of color is high (>=4), the number of shades should be small (<=3)
    // WARNING : if the number of colors is low (2 or 3), the number of shades can be high (4 or 5).
	// example 1 : setDim(2,2) will generate 2 colors (probably red and blue) with 2 shades for each colors
    // example 2 : setDim(1, 3, Qt::red, 0.1f); will generate 1 color with 3 shades excluding the red range (+/- 0.1f on a 1.0 circumference circle)

    Q_INVOKABLE static bool setDim(unsigned colorNum, unsigned ShadesNum,
					   QColor color_to_exclude=Qt::black, float exclusion_width=0.1f);

	// setDimFromColor
    Q_INVOKABLE static bool setDimFromColor(QColor /*base_color*/, unsigned /*ShadesNum*/) { return false; }

   // Secondly, retrieve the color of your choice.
   // drab = not bright. (in french : 'fade'), default is bright color
   Q_INVOKABLE static bool getColor(unsigned colorIndex, unsigned shadeIndex, QColor &c, bool drab=false);

private:
    static QVector< QVector <QColor> > s_Colors;
    static QVector< QVector <QColor> > s_DrabColors;
	static bool recalculateColors(QColor color_to_exclude=Qt::black, float exclusion_width=0.1f);
};

#endif // COLORS_GENERATOR_H
