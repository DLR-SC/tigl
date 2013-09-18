
#include "TiglViewerHUD.h"

#include "MaterialTemplate.h"

#include <osg/Matrixd>
#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osg/LightSource>
#include <osgText/Text>


TiglViewerHUD::TiglViewerHUD(){
	init();
}
TiglViewerHUD::TiglViewerHUD(const TiglViewerHUD &copy, osg::CopyOp copyop):
    osg::Camera(copy, copyop),
    _coordinateCross(copy._coordinateCross),
    _mainCamera(copy._mainCamera){}
TiglViewerHUD::~TiglViewerHUD(){
	init();
}

void TiglViewerHUD::init(){
    setProjectionMatrix(osg::Matrixd::ortho2D(-1.5, 1.5, -1.5, 1.5));
    setRenderOrder( osg::Camera::POST_RENDER, 1);
    setClearMask( GL_DEPTH_BUFFER_BIT );
    setAllowEventFocus( false );
    setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    setName("HUD");

    // TODO: check if this is necessary
    getOrCreateStateSet()->setMode( GL_BLEND,
             osg::StateAttribute::ON );

    osg::LightSource* lightSource = new osg::LightSource;
    lightSource->getLight()->setPosition(osg::Vec4d( 0.0f, 0.0f, 5.f, 1.0f )); // point light
    addChild(lightSource);
}

void TiglViewerHUD::traverse(osg::NodeVisitor &nv)
{
	// apply camera rotation to the coordinate cross
	if ( _mainCamera.valid() &&  _coordinateCross.valid() && nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR )
	{
		osg::Vec3 trans, scale;
		osg::Quat rot, orient;
		_mainCamera->getViewMatrix().decompose(trans, rot, scale, orient);

		osg::Matrix rot_mat(rot);

		// the -10 moves the cross a bit back to prevent culling at the view plane
		rot_mat.postMult(osg::Matrix::translate(-0.5,-0.5,-10));

		_coordinateCross->setMatrix(rot_mat);

		_coordinateCross->accept( nv );
	}

	osg::Camera::traverse( nv );
}

void TiglViewerHUD::setCoordinateCrossEnabled(bool enabled){
	if(enabled) {
		_coordinateCross = createCoordinateCross();
	}
	else {
		_coordinateCross = NULL;
	}
}


/**
 * creates a coordinate cross that can be displayed inside the HUD
 */
osg::MatrixTransform* TiglViewerHUD::createCoordinateCross()
{
    int objectX = 0;
    int objectY = 0;
    int objectZ = 0;

    double thickness = 0.05;
    double coneThick = 0.09;
    double length = 0.85;
    double conelen = 0.3;

    osg::Vec4Array* colors = new osg::Vec4Array();
    colors->push_back(osg::Vec4(1.0f,0.0f,0.0f,1.0f));

    osg::Cylinder* xCylinder = new osg::Cylinder(osg::Vec3(objectX+(length-conelen)/2.,objectY,objectZ), thickness, length-conelen);
    xCylinder->setRotation(osg::Quat(osg::PI_2, osg::Vec3(0,0.5f,0)));
    osg::ShapeDrawable* xC = new osg::ShapeDrawable(xCylinder);
    xC->setColor(osg::Vec4(1.0f,0.0f,0.0f,1.0f));

    osg::Cylinder* yCylinder = new osg::Cylinder(osg::Vec3(objectX,objectY+(length-conelen)/2.,objectZ), thickness, length-conelen);
    yCylinder->setRotation(osg::Quat(osg::PI_2, osg::Vec3(0.5f,0,0)));
    osg::ShapeDrawable* yC = new osg::ShapeDrawable(yCylinder);
    yC->setColor(osg::Vec4(0.0,1.0,0.0,1.0));

    osg::Cylinder* zCylinder = new osg::Cylinder(osg::Vec3(objectX,objectY,objectZ+(length-conelen)/2.), thickness, length-conelen);
    zCylinder->setRotation(osg::Quat(osg::PI, osg::Vec3(0.5f,0,0)));
    osg::ShapeDrawable* zC = new osg::ShapeDrawable(zCylinder);
    zC->setColor(osg::Vec4(0.0,0.0,1.0,1.0));

    osg::Cone* xCone = new osg::Cone(osg::Vec3(objectX+length-conelen,objectY,objectZ), coneThick, conelen);
    xCone->setRotation(osg::Quat(osg::PI_2, osg::Vec3(0,0.5f,0)));
    osg::ShapeDrawable* xO = new osg::ShapeDrawable(xCone);
    xO->setColor(osg::Vec4(1.0,0.0,0.0,1.0));

    osg::Cone* yCone = new osg::Cone(osg::Vec3(objectX,objectY+length-conelen,objectZ), coneThick, conelen);
    yCone->setRotation(osg::Quat(-osg::PI_2, osg::Vec3(0.5f,0,0)));
    osg::ShapeDrawable* yO = new osg::ShapeDrawable(yCone);
    yO->setColor(osg::Vec4(0.0,1.0,0.0,1.0));

    osg::Cone* zCone = new osg::Cone(osg::Vec3(objectX,objectY,objectZ+length-conelen), coneThick, conelen);
    //zCone->setRotation(osg::Quat(osg::PI_2, osg::Vec3(0.5f,0,0)));
    osg::ShapeDrawable* zO = new osg::ShapeDrawable(zCone);
    zO->setColor(osg::Vec4(0.0,0.0,1.0,1.0));

    osgText::Text* xText = new osgText::Text;
    xText->setPosition(osg::Vec3(objectX + 0.95,objectY,objectZ));
    xText->setAlignment(osgText::Text::CENTER_CENTER);
    xText->setText("X");
    xText->setFont("arial.ttf");
    xText->setCharacterSize(0.3);
    xText->setAxisAlignment(osgText::Text::SCREEN);

    osgText::Text* yText = new osgText::Text;
    yText->setPosition(osg::Vec3(objectX,objectY + 0.95,objectZ));
    yText->setAlignment(osgText::Text::CENTER_CENTER);
    yText->setText("Y");
    //yText->setFont("arial.ttf");
    yText->setCharacterSize(0.3);
    yText->setAxisAlignment(osgText::Text::SCREEN);

    osgText::Text* zText = new osgText::Text;
    zText->setPosition(osg::Vec3(objectX,objectY,objectZ + 0.95));
    zText->setAlignment(osgText::Text::CENTER_CENTER);
    zText->setText("Z");
    zText->setColor(osg::Vec4(0,0,0,1));
    zText->setFont("arial.ttf");
    zText->setCharacterSize(0.3);
    zText->setAxisAlignment(osgText::Text::SCREEN);

    // apply materials for correct lighting
    osg::Geode * xArrowGeode = new osg::Geode;
    xArrowGeode->addDrawable(xC);
    xArrowGeode->addDrawable(xO);
    xArrowGeode->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(RED));

    osg::Geode * yArrowGeode = new osg::Geode;
    yArrowGeode->addDrawable(yC);
    yArrowGeode->addDrawable(yO);
    yArrowGeode->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(GREEN));

    osg::Geode * zArrowGeode = new osg::Geode;
    zArrowGeode->addDrawable(zC);
    zArrowGeode->addDrawable(zO);
    zArrowGeode->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(BLUE));



    osg::Geode* centerBallGeode = new osg::Geode();
    centerBallGeode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(objectX,objectY,objectZ), 0.10f)));
    //centerBallGeode->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(WHITE));

    centerBallGeode->addDrawable(xText);
    centerBallGeode->addDrawable(yText);
    centerBallGeode->addDrawable(zText);

    osg::ref_ptr<osg::MatrixTransform> output = new osg::MatrixTransform;
    output->addChild(centerBallGeode);
    output->addChild(xArrowGeode);
    output->addChild(yArrowGeode);
    output->addChild(zArrowGeode);

    osg::StateSet* stateset = output->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::ON);

    return output.release();
}
