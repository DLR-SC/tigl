import java.util.List;

import com.sun.jna.ptr.DoubleByReference;
import com.sun.jna.ptr.IntByReference;

import de.dlr.sc.tigl.CpacsConfiguration;
import de.dlr.sc.tigl.Tigl.WSProjectionResult;
import de.dlr.sc.tigl.TiglBSpline;
import de.dlr.sc.tigl.Tigl;
import de.dlr.sc.tigl.TiglNativeInterface;
import de.dlr.sc.tigl.TiglPoint;
import de.dlr.sc.tigl.TiglSymmetryAxis;
import de.dlr.sc.tigl.TiglException;


public class JavaDemo {


	public static void main(String[] args) {
		if (args.length != 1) {
			System.err.println("Usage: tigltest <cpacs-file>");
			return;
		}
		
		String filename = args[0];
		
		System.out.println("TiGL Version: " + Tigl.getVersion());
		
		try (CpacsConfiguration config = Tigl.openCPACSConfiguration(filename, "")) {
			
			// get splines from a profile NACA0012
			List<TiglBSpline> splines = config.getProfileSplines("NACA0012");
			for (int ispl = 0; ispl < splines.size(); ++ispl) {
				TiglBSpline spl = splines.get(ispl);
				System.out.println(spl.degree + " " + spl.controlPoints.size() + " " + spl.knots.size());
			}
			
			System.out.println("fuselagecount: " + config.getFuselageCount());
			String wingUID = config.wingGetUID(1);
			
			for (int i = 1; i <= config.wingGetSectionCount(1); ++i) {
				System.out.println(config.wingGetSectionUID(1, i));
			}
			
			TiglPoint p = config.wingGetLowerPoint(1, 1, 0.5, 0.5);
			System.out.println(p);
			WSProjectionResult res = config.wingGetSegmentEtaXsi(1, p);
			System.out.println("eta/xsi: " + res.point.eta + "," + res.point.xsi);
			System.out.println("OnTop: " + res.isOnTop);
			
			System.out.println(config.wingComponentSegmentGetPoint("WING_CS1", 0.5, 0.5));
			
			Tigl.WGetPointDirectionResult result = config.wingGetUpperPointAtDirection(1, 1, 0.5, 0.5, new TiglPoint(0, 0, 1));
			System.out.println("GetPointDirection point=" + result.point);
			System.out.println("GetPointDirection error=" + result.errorDistance);

			// do some exports
			config.exportIGES("test.igs");
			config.exportSTEP("test.stp");
			config.exportWingCollada(config.wingGetUID(1), "wing.dae", 0.01);
			
			// example how to access low level tigl interface
			IntByReference wingCount = new IntByReference();
			if (TiglNativeInterface.tiglGetWingCount(config.getCPACSHandle(), wingCount) == 0) {
				System.out.println("wingcount: " + wingCount.getValue());
			}
			
			DoubleByReference mac   = new DoubleByReference();
			DoubleByReference mac_x = new DoubleByReference();
			DoubleByReference mac_y = new DoubleByReference();
			DoubleByReference mac_z = new DoubleByReference();
			if (TiglNativeInterface.tiglWingGetMAC(config.getCPACSHandle(), wingUID, mac, mac_x, mac_y, mac_z) == 0) {
				System.out.println("wing mac: "+ mac.getValue());
			}
			
			System.out.println("Ref area: " + config.wingGetReferenceArea(1, TiglSymmetryAxis.TIGL_X_Y_PLANE));
		}
		catch(TiglException err) {
			System.out.println(err.getMessage());
			System.out.println(err.getErrorCode());
			return;
		}
		
	}

}
