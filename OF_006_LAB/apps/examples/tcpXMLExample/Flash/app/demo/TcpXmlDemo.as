﻿package app.demo{	import flash.display.Sprite;	import com.touchlib.*;		import flash.display.Bitmap;	import flash.display.BitmapData;		import flash.text.*;		import flash.display.*;			import flash.events.*;	import flash.net.*;		import flash.geom.*;					public class TcpXmlDemo extends Sprite {				private var isActive:Boolean = false;		private var activeX:Number;		private var activeY:Number;				private var mouseActive:Boolean = false;						public function simpleDemo() {					trace("running");			TUIO.init( this, 'localhost', 3000, '', true );			//var subobj = new RippleCanvas();			//this.addChild(subobj);													this.addEventListener(TUIOEvent.TUIO_MOVE, this.tuioMoveHandler, false, 0, true);						this.addEventListener(TUIOEvent.TUIO_DOWN, this.tuioDownEvent, false, 0, true);									this.addEventListener(TUIOEvent.TUIO_UP, this.tuioUpEvent, false, 0, true);												this.addEventListener(TUIOEvent.TUIO_OVER, this.tuioRollOverHandler, false, 0, true);												this.addEventListener(TUIOEvent.TUIO_OUT, this.tuioRollOutHandler, false, 0, true);						this.addEventListener(MouseEvent.MOUSE_MOVE, this.mouseMoveHandler, false, 0, true);												this.addEventListener(MouseEvent.MOUSE_DOWN, this.mouseDownEvent, false, 0, true);																		this.addEventListener(MouseEvent.MOUSE_UP, this.mouseUpEvent, false, 0, true);				this.addEventListener(MouseEvent.ROLL_OVER, this.mouseRollOverHandler, false, 0, true);			this.addEventListener(MouseEvent.ROLL_OVER, this.mouseRollOutHandler, false, 0, true);						this.addEventListener(Event.ENTER_FRAME, this.frameUpdate, false, 0, true);										}		function beginDrag()		{			isActive = true;					}				function endDrag()		{			if(isActive)			{				isActive = false;						}			mouseActive = false;							}						public function getActive():Boolean		{			return isActive;		}				function frameUpdate(e:Event)		{			if(isActive)			{				if(mouseActive)				{					activeX = this.mouseX;					activeY = this.mouseY;				}				pinkBox.x = activeX;				pinkBox.y = activeY;			}		}				public function tuioDownEvent(e:TUIOEvent)		{					trace(TUIO.returnBlobs());						TUIO.listenForObject(e.ID, this);			beginDrag();						e.stopPropagation();		}		public function tuioUpEvent(e:TUIOEvent)		{					endDrag();					e.stopPropagation();		}				public function tuioMoveHandler(e:TUIOEvent)		{			if(isActive)			{				var tuioobj:TUIOObject = TUIO.getObjectById(e.ID);											var localPt:Point = globalToLocal(new Point(tuioobj.x, tuioobj.y));																		activeX = localPt.x;				activeY = localPt.y;			}			e.stopPropagation();					}				public function tuioRollOverHandler(e:TUIOEvent)		{					}				public function tuioRollOutHandler(e:TUIOEvent)		{			e.stopPropagation();							}							public function mouseDownEvent(e:MouseEvent)		{					mouseActive = true;			beginDrag();		}				public function mouseUpEvent(e:MouseEvent)		{					endDrag();		}				public function mouseMoveHandler(e:MouseEvent)		{			if(isActive)			{				activeX = this.mouseX;				activeY = this.mouseY;						}		}				public function mouseRollOverHandler(e:MouseEvent)		{		}				public function mouseRollOutHandler(e:MouseEvent)		{//			sliderStopDrag();							}						}}