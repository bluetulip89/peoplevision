﻿/******************************************************************	CUSTOM PERSON	a simple class that moves + sizes a square 	based on a CYA person	******************************************************************/package{	import com.lab.CYAPerson;	import com.lab.CYAEvent;	import flash.display.MovieClip;	import flash.display.Shape;	import flash.text.TextField;		public class CustomPerson extends MovieClip	{		var person:CYAPerson;		var infoMC:MovieClip;				var contourShape:Shape;				public function CustomPerson( _person:CYAPerson )		{			super();			person = _person;			addChild(person);			contourShape = new Shape();			addChild(contourShape);						// listen for the person update event, which is dispatched			// every time CYA gets new info about a person object			person.addEventListener( CYAEvent.PERSON_UPDATED, update );			//setBackground();			update();		}				public function setBackground (frame:uint=100)		{			if (frame < bg_mc.totalFrames) bg_mc.gotoAndStop(frame);			else bg_mc.gotoAndStop(Math.floor(bg_mc.totalFrames*Math.random()))		}				// update the x, y, width, height, and info text				public function update( e:CYAEvent = null )		{			x = person.x;			y = person.y;			centerpoint.x = person.centroid.x - x;			centerpoint.y = person.centroid.y - y;			id_txt.text = String(person.id);			age_txt.text = String(person.age);			bg_mc.width = person.realWidth;				bg_mc.height = person.realHeight;						//move + mask bounding boxes			boundingL_mc.mask_mc.height = bg_mc.height;			boundingT_mc.mask_mc.width 	= bg_mc.width;			boundingR_mc.x 		= bg_mc.width;			boundingR_mc.mask_mc.height = bg_mc.height;			boundingB_mc.y = bg_mc.height;			boundingB_mc.mask_mc.width = bg_mc.width;						//contour			contourShape.graphics.clear();			contourShape.graphics.beginFill(0xffffff,0);			contourShape.graphics.lineStyle(1,0xffffff,.5);			if (person.contours.length > 0) contourShape.graphics.moveTo(person.contours[0].x-x, person.contours[0].y-y);			for (var i:uint = 0; i<person.contours.length; i++){				contourShape.graphics.lineTo(person.contours[i].x-x, person.contours[i].y-y);			}			contourShape.graphics.endFill();					}	}}