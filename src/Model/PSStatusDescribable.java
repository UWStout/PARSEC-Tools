package edu.uwstout.berriers.PSHelper.Model;

public interface PSStatusDescribable {

	public enum Status {
		// Not yet initialized
		UNKNOWN("Unknown"),
		
		// Automatically identified status (sequential)
		UNPROCESSSED("Unprocessed"),
		RAW_PROCESSING_DONE("Images Ready"),
		ALIGNMENT_DONE("Images Aligned"),
		POINT_CLOUD_DONE("Dense Cloud Done"),
		MODEL_GEN_DONE("Model Generated"),
		TEXTURE_GEN_DONE("Complete"),

		// Manually assigned status (rejection or approval)
		NEEDS_EXPOSURE_REDO("Need to redo Raw Image Exposure"),
		NEEDS_ALLIGNMENT_REDO("Need to redo Image Alignment"),
		NEEDS_POINT_CLOUD_REDO("Need to redo Dense Point Cloud"),
		NEEDS_MODEL_GEN_REDO("Need to redo Model Generation"),
		NEEDS_TEXTURE_GEN_REDO("Need to redo Texture Generation"),
		NEEDS_GEOMETRY_TOUCHUP("Needs Geometry Touchup by Modeler"),
		NEEDS_TEXTURE_TOUCHUP("Needs Texture Touchup by Modeler"),
		FINAL_APPROVAL("Approved!");
		
		// Manage string description
		private final String text;
		Status(String text) { this.text = text; }		
		public String toString() { return text; }
	}

	public String describeImageAlignPhase();
	public byte getAlignPhaseStatus();
	
	public String describeDenseCloudPhase();
	public byte getDenseCloudPhaseStatus();
	public int getDenseCloudDepthImages();

	public String describeModelGenPhase();
	public byte getModelGenPhaseStatus();
	public long getModelFaceCount();
	public long getModelVertexCount();

	public String describeTextureGenPhase();
	public byte getTextureGenPhaseStatus();
}
