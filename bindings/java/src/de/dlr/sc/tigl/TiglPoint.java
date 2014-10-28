package de.dlr.sc.tigl;
/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/



/**
 * 3D-Point container class with getter and setter for simple 3d points.
 * 
 * @author Markus Litz
 *
 */
public class TiglPoint {

	/** X-Coordinate of the point */
	private double x;

	/** Y-Coordinate of the point */
	private double y;

	/** Z-Coordinate of the point */
	private double z;
	
	/**
	 * Default constructor, setting everything to zero.
	 */
	public TiglPoint(){
		x = 0.0;
		y = 0.0;
		z = 0.0;
	}

	/**
	 * Constructor with parameters for x, y and z.
	 * @param myX
	 * @param myY
	 * @param myZ
	 */
	public TiglPoint(double myX, double myY, double myZ){
		x = myX;
		y = myY;
		z = myZ;
	}
	
	
	/**
	 * @return the x
	 */
	public double getX() {
		return x;
	}


	/**
	 * @param x the x to set
	 */
	public void setX(double x) {
		this.x = x;
	}


	/**
	 * @return the y
	 */
	public double getY() {
		return y;
	}


	/**
	 * @param y the y to set
	 */
	public void setY(double y) {
		this.y = y;
	}


	/**
	 * @return the z
	 */
	public double getZ() {
		return z;
	}


	/**
	 * @param z the z to set
	 */
	public void setZ(double z) {
		this.z = z;
	}
	
	public String toString() {
		return "(" + this.x + ", " + this.y + ", " + this.z + ")";
	}
}
