#include "colors_generator.h"
#include <gqtl_log.h>

// Version 29 july 2009 : 17h26

QVector< QVector <QColor> > ColorsGenerator::s_Colors;
QVector< QVector <QColor> > ColorsGenerator::s_DrabColors;

// HSV to RGB translation
// input  H,S,B in [0.f,1.f] range
int hsv2rgb(float H, float S, float V, int &R, int &G, int &B)
{
	float Hf=(H-(int)H)*6;
	int Hi= (int)Hf;

	float f = Hf-Hi;
	float p = V*(1-S);
	float q = V*(1-f*S);
	float t = V*(1-(1-f)*S);

	switch(Hi)
	{
	    case 0: R=(int)(V*255); G=(int)(t*255);  B=(int)(p*255); break;
	    case 1: R=(int)(q*255); G=(int)(V*255);  B=(int)(p*255); break;
	    case 2: R=(int)(p*255); G=(int)(V*255);  B=(int)(t*255); break;
	    case 3: R=(int)(p*255); G=(int)(q*255);  B=(int)(V*255); break;
	    case 4: R=(int)(t*255); G=(int)(p*255);  B=(int)(V*255); break;
	    case 5: R=(int)(V*255); G=(int)(p*255);  B=(int)(q*255); break;
	}

	return (R<<16)|(G<<8)|B;
}

// Recalculate all the colors following the size of dimensions
bool ColorsGenerator::recalculateColors(QColor color_to_exclude, float exclusion_width)
{
    if ( s_Colors.size()==0 || s_Colors[0].size()==0)
		return false;

	// color_separator = (number_of_shade/number_of_color) + (number_of_color/number_of_shades)

	float num_shades=s_Colors[0].size();
	float num_colors=s_Colors.size();

	/*
	float m_color_separation =	s_Colors[0].size()/s_Colors.size()
								+ s_Colors.size()/s_Colors[0].size(); // 0.5f;
	*/
	float m_color_separation =	num_shades/num_colors + num_colors/num_shades;

	if (color_to_exclude!=Qt::black)
		m_color_separation =	num_shades/(num_colors+1.f) + (num_colors+1.f)/(num_shades);

    float HueRangePerCols = 1.f/((float) s_Colors.size());
	if (color_to_exclude!=Qt::black)	// if there is a color to exclude, we have to reduce the available range
		HueRangePerCols = 1.f/((float) s_Colors.size()+1.0f);

	// use QColor::toHsv() ??
	// should return a real between 0 and 1 !
	//GSLOG(SYSLOG_SEV_DEBUG, "%d %f", color_to_exclude.hue(), color_to_exclude.hueF());
	float HueToXclude=((float)color_to_exclude.hueF()); // will be negative if achromatic color (black...)

	float HueRangeForAllColors=1.0f;
	if (color_to_exclude!=Qt::black)
		HueRangeForAllColors=1.0f-2.f*exclusion_width;

    GSLOG(SYSLOG_SEV_DEBUG,
          QString("ColorsGenerator::recalculateColors: "
                  "HueRangeForAllColors=%1 "
                  "HueRangePerCols=%2 "
                  "color_sep=%3 HueToExclude=%4 exclusion_width=%5").
          arg(HueRangeForAllColors).
          arg(HueRangePerCols).
          arg(m_color_separation).
          arg(HueToXclude).
          arg(exclusion_width).toLatin1().constData());

	for (int i=0; i < s_Colors.size(); ++i)	// for each colors
    {
		// 0° = red 60°=yellow 120°=green 180°=cyan 240°=blue 300°=magenta ...
		float Hue=((float)i)/((float) s_Colors.size())*HueRangeForAllColors;  // between 0 and 1.0
		//if (color_to_exclude!=Qt::black)
		//	Hue=((float)i)/((float) s_Colors.size()+1.0f);
		if (HueToXclude>=0.f)
			Hue+=(HueToXclude+exclusion_width);

		for (int j=0; j< s_Colors[i].size(); ++j)	// for each shades
		{
			int r,g,b;
			float _H=Hue; 	//float H=((float)rand()/RAND_MAX);

			_H+=(((float)j/((float)s_Colors[i].size()+m_color_separation))*HueRangePerCols);
			float S=1.f; // Saturation : couleur vif ou pale
			float V=1.f; // Value (dark or light)
			hsv2rgb(_H,S,V, r,g,b);
			s_Colors[i][j]=QColor(r,g,b);
			hsv2rgb(_H, 0.55f, V, r,g,b);
			s_DrabColors[i][j]=QColor(r,g,b);
		}
    }
    return true;
}

// set dimensions
bool ColorsGenerator::setDim(unsigned colorsNum, unsigned ShadesNum,
							 QColor color_to_exclude, float exclusion_width)
{
    if ( colorsNum==0 || ShadesNum==0 )
		return false;

	if (colorsNum==(unsigned)s_Colors.size() && s_Colors[0].size()==(int)ShadesNum)
		return false;

    s_Colors.resize(colorsNum);
    s_DrabColors.resize(colorsNum);

    for (int i=0; i<s_Colors.size(); i++)
    {
		s_DrabColors[i].resize( (int)ShadesNum );
		s_Colors[i].resize((int)ShadesNum);
    }

	return ColorsGenerator::recalculateColors(color_to_exclude, exclusion_width);
}

// get the colors
bool ColorsGenerator::getColor(unsigned colorIndex, unsigned shadeIndex, QColor &c, bool drab)
{
	if ( colorIndex >= (unsigned)s_Colors.size() ||  shadeIndex >= (unsigned)s_Colors[colorIndex].size() )
	return false;

    if (drab)
     c=s_DrabColors[colorIndex][shadeIndex];
    else
     c=s_Colors[colorIndex][shadeIndex];

    return true;
}
