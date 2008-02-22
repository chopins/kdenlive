/***************************************************************************
                          parameterplotter.cpp  -  description
                             -------------------
    begin                : Feb 15 2008
    copyright            : (C) 2008 by Marco Gittler
    email                : g.marco@freenet.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "parameterplotter.h"
#include <QVariant>
#include <KPlotObject>
#include <QMouseEvent>
#include <KDebug>
#include <KPlotPoint>

ParameterPlotter::ParameterPlotter (QWidget *parent):KPlotWidget (parent){
	setAntialiasing(true);
	setLeftPadding(20);
	setRightPadding(10);
	setTopPadding(10);
	setBottomPadding(20);
	movepoint=NULL;
	colors << Qt::white << Qt::red << Qt::green << Qt::blue << Qt::magenta << Qt::gray << Qt::cyan;
	maxy=0;
	m_moveX=false;
	m_moveY=true;
	m_moveTimeline=true;
	m_newPoints=false;
	activeIndexPlot=-1;
}
/*
    <name>Lines</name>
    <description>Lines from top to bottom</description>
    <author>Marco Gittler</author>
    <properties tag="lines" id="lines" />
    <parameter default="5" type="constant" value="5" min="0" name="num" max="255" >
      <name>Num</name>
    </parameter>
    <parameter default="4" type="constant" value="4" min="0" name="width" max="255" >
      <name>Width</name>
    </parameter>
  </effect>

*/
void ParameterPlotter::setPointLists(const QDomElement& d,int startframe,int endframe){
	
	//QListIterator <QPair <QString, QMap< int , QVariant > > > nameit(params);
	itemParameter=d;
	QDomNodeList namenode = d.elementsByTagName("parameter");
	
	int max_y=0;
	removeAllPlotObjects ();
	parameterNameList.clear();
	plotobjects.clear();
	

	for (int i=0;i< namenode.count() ;i++){
		KPlotObject *plot=new KPlotObject(colors[plotobjects.size()%colors.size()]);
		plot->setShowLines(true);
		//QPair<QString, QMap< int , QVariant > > item=nameit.next();
		QDomNode pa=namenode.item(i);
		QDomNode na=pa.firstChildElement("name");
		
		parameterNameList << na.toElement().text();
		
		
		max_y=pa.attributes().namedItem("max").nodeValue().toInt();
		int val=pa.attributes().namedItem("value").nodeValue().toInt();
		plot->addPoint((i+1)*20,val);
		/*TODO keyframes
		while (pointit.hasNext()){
			pointit.next();
			plot->addPoint(QPointF(pointit.key(),pointit.value().toDouble()),item.first,1);
			if (pointit.value().toInt() >maxy)
				max_y=pointit.value().toInt();
		}*/
		plotobjects.append(plot);
	}
	maxx=endframe;
	maxy=max_y;
	setLimits(0,endframe,0,maxy+10);
	addPlotObjects(plotobjects);

}

void ParameterPlotter::createParametersNew(){
	
	QList<KPlotObject*> plotobjs=plotObjects();
	if (plotobjs.size() != parameterNameList.size() ){
		kDebug() << "ERROR size not equal";
	}
	QDomNodeList namenode = itemParameter.elementsByTagName("parameter");
	for (int i=0;i<namenode.count() ;i++){
		QList<KPlotPoint*> points=plotobjs[i]->points();
		QDomNode pa=namenode.item(i);
		
		
		
		
		
		
		QMap<int,QVariant> vals;
		foreach (KPlotPoint *o,points){
			//vals[o->x()]=o->y();
			pa.attributes().namedItem("value").setNodeValue(QString::number(o->y()));
		}
		QPair<QString,QMap<int,QVariant> > pair("contrast",vals);
		//ret.append(pair);
	}
	
	emit parameterChanged(itemParameter);
	
}


void ParameterPlotter::mouseMoveEvent ( QMouseEvent * event ) {
	
	if (movepoint!=NULL){
		QList<KPlotPoint*> list=   pointsUnderPoint (event->pos()-QPoint(leftPadding(), topPadding() )  ) ;
		int i=0,j=-1;
		foreach (KPlotObject *o, plotObjects() ){
			QList<KPlotPoint*> points=o->points();
			for(int p=0;p<points.size();p++){
				if (points[p]==movepoint){
					QPoint delta=event->pos()-oldmousepoint;
					if (m_moveY)
					movepoint->setY(movepoint->y()-delta.y()*dataRect().height()/pixRect().height() );
					if (p>0 && p<points.size()-1){
						double newx=movepoint->x()+delta.x()*dataRect().width()/pixRect().width();
						if ( newx>points[p-1]->x() && newx<points[p+1]->x() && m_moveX)
							movepoint->setX(movepoint->x()+delta.x()*dataRect().width()/pixRect().width() );
					}
					if (m_moveTimeline && (m_moveX|| m_moveY) )
						emit updateFrame(0);
					replacePlotObject(i,o);
					oldmousepoint=event->pos();
				}
			}
			i++;
		}
		createParametersNew();
	}
}

void ParameterPlotter::replot(const QString & name){
	//removeAllPlotObjects();
	int i=0;
	bool drawAll=name.isEmpty() || name=="all";
	activeIndexPlot=-1;
	foreach (KPlotObject* p,plotObjects() ){
		p->setShowPoints(drawAll || parameterNameList[i]==name);
		p->setShowLines(drawAll || parameterNameList[i]==name);
		if ( parameterNameList[i]==name )
			activeIndexPlot = i;
		replacePlotObject(i++,p);
	}
}

void ParameterPlotter::mousePressEvent ( QMouseEvent * event ) {
	//topPadding and other padding can be wrong and this (i hope) will be correctet in newer kde versions
	QPoint inPlot=event->pos()-QPoint(leftPadding(), topPadding() );
	QList<KPlotPoint*> list=   pointsUnderPoint (inPlot ) ;
	if (event->button()==Qt::LeftButton){
		if (list.size() > 0){
			movepoint=list[0];
			oldmousepoint=event->pos();
		}else{
			if (m_newPoints && activeIndexPlot>=0){
				//setup new points
				KPlotObject* p=plotObjects()[activeIndexPlot];
				QList<KPlotPoint*> points=p->points();
				QList<QPointF> newpoints;
				
				double newx=inPlot.x()*dataRect().width()/pixRect().width();
				double newy=(height()-inPlot.y()-bottomPadding()-topPadding() )*dataRect().height()/pixRect().height();
				bool inserted=false;
				foreach (KPlotPoint* pt,points){
					if (pt->x() >newx && !inserted){
						newpoints.append(QPointF(newx,newy));
						inserted=true;
					}
					newpoints.append(QPointF(pt->x(),pt->y()));
				}
				p->clearPoints();
				foreach (QPointF qf, newpoints ){
					p->addPoint(qf);
				}
				replacePlotObject(activeIndexPlot,p);
			}
			movepoint=NULL;
		}
	}else if (event->button()==Qt::LeftButton){
		//menu for deleting or exact setup of point
	}
}

void ParameterPlotter::setMoveX(bool b){
	m_moveX=b;
}

void ParameterPlotter::setMoveY(bool b){
	m_moveY=b;
}

void ParameterPlotter::setMoveTimeLine(bool b){
	m_moveTimeline=b;
}

void ParameterPlotter::setNewPoints(bool b){
	m_newPoints=b;
}

bool ParameterPlotter::isMoveX(){
	return m_moveX;
}

bool ParameterPlotter::isMoveY(){
	return m_moveY;
}

bool ParameterPlotter::isMoveTimeline(){
	return m_moveTimeline;
}

bool ParameterPlotter::isNewPoints(){
	return m_newPoints;
}
