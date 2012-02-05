#include "Patch.hpp"
#include "Rectoid.hpp"
#include "RenderUtils.hpp"

namespace rendering {

/// Rectoid

void Rectoid::translate(const QVector3D &t)
{
	const QVector3D st = t * _scale;
	for (int i = 0; i < parts.count(); ++i)
		parts[i]->translate(st);
}
void Rectoid::rotate(qreal deg, QVector3D axis)
{
	for (int i = 0; i < parts.count(); ++i)
		parts[i]->rotate(deg, axis);
}

void Rectoid::setColor(QColor c)
{
	for (int i = 0; i < parts.count(); ++i)
		qSetColor(parts[i]->faceColor, c);
}

/// RectPrism

RectPrism::RectPrism(Geometry *g, qreal scale, qreal widthUn, qreal heightUn, qreal depthUn)
: Rectoid(scale)
{
	// scale measurements
	qreal width  = widthUn  * _scale;
	qreal height = heightUn * _scale;
	qreal depth  = depthUn  * _scale;

	enum { bl, br, tr, tl };
	Patch *fb = new Patch(g);
	fb->setSmoothing(Patch::Faceted);

	// front face
	QVector<QVector3D> r(4);
	r[br].setX(width);
	r[tr].setX(width);
	r[tr].setY(height);
	r[tl].setY(height);
	QVector3D adjToCenter(-width / 2.0, -height / 2.0, depth / 2.0);
	for (int i = 0; i < 4; ++i)
		r[i] += adjToCenter;
	fb->addQuad(r[bl], r[br], r[tr], r[tl]);

	// back face
	QVector<QVector3D> s = extrude(r, depth);
	fb->addQuad(s[tl], s[tr], s[br], s[bl]);

	// side faces
	Patch *sides = new Patch(g);
	sides->setSmoothing(Patch::Faceted);
	sides->addQuad(s[bl], s[br], r[br], r[bl]);
	sides->addQuad(s[br], s[tr], r[tr], r[br]);
	sides->addQuad(s[tr], s[tl], r[tl], r[tr]);
	sides->addQuad(s[tl], s[bl], r[bl], r[tl]);

	parts << fb << sides;
}

/// RectTorus

RectTorus::RectTorus(Geometry *g, qreal scale, qreal iRad, qreal oRad, qreal depth, int k)
: Rectoid(scale)
{
	QVector<QVector3D> inside;
	QVector<QVector3D> outside;
	for (int i = 0; i < k; ++i) {
		qreal angle = (i * 2 * M_PI) / k;
		inside << QVector3D(iRad * qSin(angle), iRad * qCos(angle), depth / 2.0);
		outside << QVector3D(oRad * qSin(angle), oRad * qCos(angle), depth / 2.0);
	}
	inside << QVector3D(0.0, iRad, 0.0);
	outside << QVector3D(0.0, oRad, 0.0);
	QVector<QVector3D> in_back = extrude(inside, depth);
	QVector<QVector3D> out_back = extrude(outside, depth);

	// Create front, back and sides as separate patches so that smooth normals
	// are generated for the curving sides, but a faceted edge is created between
	// sides and front/back
	Patch *front = new Patch(g);
	for (int i = 0; i < k; ++i)
		front->addQuad(outside[i], inside[i],
				inside[(i + 1) % k], outside[(i + 1) % k]);
	Patch *back = new Patch(g);
	for (int i = 0; i < k; ++i)
		back->addQuad(in_back[i], out_back[i],
				out_back[(i + 1) % k], in_back[(i + 1) % k]);
	Patch *is = new Patch(g);
	for (int i = 0; i < k; ++i)
		is->addQuad(in_back[i], in_back[(i + 1) % k],
				inside[(i + 1) % k], inside[i]);
	Patch *os = new Patch(g);
	for (int i = 0; i < k; ++i)
		os->addQuad(out_back[(i + 1) % k], out_back[i],
				outside[i], outside[(i + 1) % k]);
	parts << front << back << is << os;
}

} // \namespace rendering