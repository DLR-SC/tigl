package de.dlr.sc.tiglviewer.android;


import android.graphics.PointF;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import de.dlr.sc.tiglviewer.android.TiglViewerActivity.navType;

public class EGLTouchListener implements OnTouchListener {
    enum moveTypes { NONE , DRAG, MDRAG, ZOOM ,ACTUALIZE}

    private static final String TAG = "TiGL Viewer";
    
    private moveTypes mode = moveTypes.NONE;
    private navType navMode = navType.ROTATE;
    private boolean moveOrpick = false;
    
    PointF oneFingerOrigin = new PointF(0,0);
    PointF twoFingerOrigin = new PointF(0,0);
    float distanceOrigin = 0.f;
    
      
    int screenDensity; 
    public EGLTouchListener(int screenDensity)
    {
        this.screenDensity = screenDensity;
    }
    public void setNavMode(TiglViewerActivity.navType type) {
        navMode = type;
    }
    
    @Override
    public boolean onTouch(View arg0, MotionEvent event) {
        int n_points = event.getPointerCount();
        int action = event.getAction() & MotionEvent.ACTION_MASK;
        switch (n_points) {
        case 2:
            moveOrpick = true;
            switch (action) {
            case MotionEvent.ACTION_POINTER_DOWN:
                switch (mode) {
                case DRAG:
                    if (navMode == navType.ROTATE) {
                        TiGLViewerNativeLib.mouseButtonReleaseEvent(
                                event.getX(0), event.getY(0), 1, 1);
                    } else {
                        TiGLViewerNativeLib.mouseButtonReleaseEvent(
                                event.getX(0), event.getY(0), 2, 1);
                    }
                    break;

                default:
                    break;
                }

                mode = moveTypes.ZOOM;
                distanceOrigin = distance(event);
                twoFingerOrigin.x = event.getX(1);
                twoFingerOrigin.y = event.getY(1);
                oneFingerOrigin.x = event.getX(0);
                oneFingerOrigin.y = event.getY(0);

                TiGLViewerNativeLib.mouseMoveEvent(oneFingerOrigin.x,
                        oneFingerOrigin.y, 1);
                TiGLViewerNativeLib.mouseButtonPressEvent(oneFingerOrigin.x,
                        oneFingerOrigin.y, 3, 1);
                TiGLViewerNativeLib.mouseMoveEvent(oneFingerOrigin.x,
                        oneFingerOrigin.y, 1);

            case MotionEvent.ACTION_MOVE:
                float distance = distance(event);
                float result = distance - distanceOrigin;
                distanceOrigin = distance;

                if (result > 1 || result < -1) {
                    oneFingerOrigin.y = oneFingerOrigin.y + result;
                    TiGLViewerNativeLib.mouseMoveEvent(oneFingerOrigin.x,
                            oneFingerOrigin.y, 1);
                }

                break;

            case MotionEvent.ACTION_POINTER_UP:
                mode = moveTypes.NONE;
                TiGLViewerNativeLib.mouseButtonReleaseEvent(oneFingerOrigin.x,
                        oneFingerOrigin.y, 3, 1);
                break;

            case MotionEvent.ACTION_UP:
                mode = moveTypes.NONE;
                TiGLViewerNativeLib.mouseButtonReleaseEvent(oneFingerOrigin.x,
                        oneFingerOrigin.y, 3, 1);
                break;

            default:
                Log.e(TAG, "2 point Action not captured");
            }
            break;

        case 1:
            switch (action) {
            case MotionEvent.ACTION_DOWN:
                mode = moveTypes.DRAG;

                TiGLViewerNativeLib.mouseMoveEvent(event.getX(0),
                        event.getY(0), 1);
                if (navMode == navType.ROTATE)
                    TiGLViewerNativeLib.mouseButtonPressEvent(event.getX(0),
                            event.getY(0), 1, 1);
                else
                    TiGLViewerNativeLib.mouseButtonPressEvent(event.getX(0),
                            event.getY(0), 2, 1);

                    oneFingerOrigin.x = event.getX(0);
                oneFingerOrigin.y = event.getY(0);
                break;

            case MotionEvent.ACTION_CANCEL:
                switch (mode) {
                case DRAG:
                    TiGLViewerNativeLib.mouseMoveEvent(event.getX(0),
                            event.getY(0), 1);
                    if (navMode == navType.ROTATE)
                        TiGLViewerNativeLib.mouseButtonReleaseEvent(
                                event.getX(0), event.getY(0), 1, 1);
                    else
                        TiGLViewerNativeLib.mouseButtonReleaseEvent(
                                event.getX(0), event.getY(0), 2, 1);
                        break;

                default:
                    Log.e(TAG,
                            "There has been an anomaly in touch input 1point/action");
                }
                mode = moveTypes.NONE;
                break;

            case MotionEvent.ACTION_MOVE:
                moveOrpick=true;
                TiGLViewerNativeLib.mouseMoveEvent(event.getX(0),
                        event.getY(0), 1);

                oneFingerOrigin.x = event.getX(0);
                oneFingerOrigin.y = event.getY(0);

                break;

            case MotionEvent.ACTION_UP:
                if (!moveOrpick) {
                    TiGLViewerNativeLib.pickEvent(event.getX(0), event.getY(0), 1);
                }
                else {
                    moveOrpick = false;
                }
                switch (mode) {
                case DRAG:
                    if (navMode == navType.ROTATE)
                        TiGLViewerNativeLib.mouseButtonReleaseEvent(
                                event.getX(0), event.getY(0), 1, 1);
                    else
                        TiGLViewerNativeLib.mouseButtonReleaseEvent(
                                event.getX(0), event.getY(0), 2, 1);
                        break;
                default:
                    Log.e(TAG,
                            "There has been an anomaly in touch input 1 point/action");
                }
                mode = moveTypes.NONE;
                break;

            default:
                Log.e(TAG, "1 point Action not captured");
            }
            break;

        }

        return true;
    }
    
    private float distance(MotionEvent event) {
        assert(event.getPointerCount() >= 2);
        float x = event.getX(0) - event.getX(1);
        float y = event.getY(0) - event.getY(1);
        return (float)Math.sqrt(x * x + y * y);
    }
}
