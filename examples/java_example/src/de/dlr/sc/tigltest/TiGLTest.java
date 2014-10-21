package de.dlr.sc.tigltest;

import java.util.ArrayList;

import com.sun.jna.ptr.DoubleByReference;
import com.sun.jna.ptr.IntByReference;

import de.dlr.sc.tigl.TiglBSpline;
import de.dlr.sc.tigl.TiglInterface;
import de.dlr.sc.tigl.TiglNativeInterface;
import de.dlr.sc.tigl.TiglPoint;
import de.dlr.sc.tigl.TiglSymmetryAxis;
import de.dlr.sc.tigl.TiglInterface.TiglException;


public class TiGLTest {


	public static void main(String[] args) {
		TiglInterface tigl = new TiglInterface();
		
		System.out.println(tigl.getVersion());
		
		try {
			tigl.openCPACSConfiguration("d:\\src\\tiglsplineexample\\simpletest.cpacs.xml", "");
		}
		catch(TiglException err) {
			System.out.println(err.getMessage());
			System.out.println(err.getErrorCode());
			return;
		}
		
		ArrayList<TiglBSpline> splines = tigl.getProfileSplines("NACA0012");
		for (int ispl = 0; ispl < splines.size(); ++ispl) {
			TiglBSpline spl = splines.get(ispl);
			System.out.println(spl.degree + " " + spl.controlPoints.size() + " " + spl.knots.size());
		}
		
		System.out.println("fuselagecount: " + tigl.getFuselageCount());
		String wingUID = tigl.wingGetUID(1);
		
		TiglPoint p = tigl.wingGetLowerPoint(1, 1, 0.5, 0.5);
		System.out.println(p);
		
		System.out.println(tigl.wingComponentSegmentGetPoint("WING_CS1", 0.5, 0.5));

		// do some exports
		tigl.exportIGES("test.igs");
		tigl.exportSTEP("test.stp");
		
		// example how to access low level tigl interface
		IntByReference wingCount = new IntByReference();
		if (TiglNativeInterface.tiglGetWingCount(tigl.getCPACSHandle(), wingCount) == 0) {
			System.out.println("wingcount: " + wingCount.getValue());
		}
		
		DoubleByReference mac = new DoubleByReference();
		DoubleByReference mac_x = new DoubleByReference();
		DoubleByReference mac_y = new DoubleByReference();
		DoubleByReference mac_z = new DoubleByReference();
		if (TiglNativeInterface.tiglWingGetMAC(tigl.getCPACSHandle(), wingUID, mac, mac_x, mac_y, mac_z) == 0) {
			System.out.println("wing mac: "+ mac.getValue());
		}
		
		System.out.println("Ref area: " + tigl.wingGetReferenceArea(1, TiglSymmetryAxis.TIGL_X_Y_PLANE));
		
		tigl.closeDocument();
		
	}

}
