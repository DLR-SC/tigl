package de.dlr.sc.tigl;

public class TiglException extends Exception{

	private static final long serialVersionUID = 1L;
	
	private TiglReturnCode errorCode = TiglReturnCode.TIGL_SUCCESS;
	private String methodName = "";
	
	public TiglException(String method, TiglReturnCode errorcode) {
		this.errorCode = errorcode;
		this.methodName = method;
	}
	
	public int getErrorCode() {
		return errorCode.getValue();
	}
	
	public String getLocation() {
		return methodName;
	}
	
	@Override
	public String getMessage() {
		return "TiglException: " + methodName  
				+ ": returned " + TiglNativeInterface.tiglGetErrorString(errorCode.getValue()) 
				+ "(" + errorCode.getValue() + ")";
	}
}