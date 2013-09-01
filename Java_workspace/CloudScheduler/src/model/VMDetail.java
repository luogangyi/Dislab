package model;

public class VMDetail {

	protected int ID;
	protected String VMNAME;
	protected String OS;
	protected int DISK;
	protected int RAM;
	protected int USERID;
	protected int DiskType;
	protected int IsParaVirt;
	protected int IsAutoScaling;
	protected String VirtType;
	protected int Status;
	protected String Host;
	protected int GroupID;
	protected int GroupInnerID;
	
	protected MACIP mapip;
	/**
	 * @return the iD
	 */
	public int getID() {
		return ID;
	}
	/**
	 * @param iD the iD to set
	 */
	public void setID(int iD) {
		ID = iD;
	}
	/**
	 * @return the vMNAME
	 */
	public String getVMNAME() {
		return VMNAME;
	}
	/**
	 * @param vMNAME the vMNAME to set
	 */
	public void setVMNAME(String vMNAME) {
		VMNAME = vMNAME;
	}
	/**
	 * @return the oS
	 */
	public String getOS() {
		return OS;
	}
	/**
	 * @param oS the oS to set
	 */
	public void setOS(String oS) {
		OS = oS;
	}
	/**
	 * @return the dISK
	 */
	public int getDISK() {
		return DISK;
	}
	/**
	 * @param dISK the dISK to set
	 */
	public void setDISK(int dISK) {
		DISK = dISK;
	}
	/**
	 * @return the rAM
	 */
	public int getRAM() {
		return RAM;
	}
	/**
	 * @param rAM the rAM to set
	 */
	public void setRAM(int rAM) {
		RAM = rAM;
	}
	/**
	 * @return the uSERID
	 */
	public int getUSERID() {
		return USERID;
	}
	/**
	 * @param uSERID the uSERID to set
	 */
	public void setUSERID(int uSERID) {
		USERID = uSERID;
	}
	/**
	 * @return the diskType
	 */
	public int getDiskType() {
		return DiskType;
	}
	/**
	 * @param diskType the diskType to set
	 */
	public void setDiskType(int diskType) {
		DiskType = diskType;
	}
	/**
	 * @return the isParaVirt
	 */
	public int getIsParaVirt() {
		return IsParaVirt;
	}
	/**
	 * @param isParaVirt the isParaVirt to set
	 */
	public void setIsParaVirt(int isParaVirt) {
		IsParaVirt = isParaVirt;
	}
	/**
	 * @return the isAutoScaling
	 */
	public int getIsAutoScaling() {
		return IsAutoScaling;
	}
	/**
	 * @param isAutoScaling the isAutoScaling to set
	 */
	public void setIsAutoScaling(int isAutoScaling) {
		IsAutoScaling = isAutoScaling;
	}
	/**
	 * @return the virtType
	 */
	public String getVirtType() {
		return VirtType;
	}
	/**
	 * @param virtType the virtType to set
	 */
	public void setVirtType(String virtType) {
		VirtType = virtType;
	}
	/**
	 * @return the status
	 */
	public int getStatus() {
		return Status;
	}
	/**
	 * @param status the status to set
	 */
	public void setStatus(int status) {
		Status = status;
	}
	/**
	 * @return the host
	 */
	public String getHost() {
		return Host;
	}
	/**
	 * @param host the host to set
	 */
	public void setHost(String host) {
		Host = host;
	}
	/**
	 * @return the groupID
	 */
	public int getGroupID() {
		return GroupID;
	}
	/**
	 * @param groupID the groupID to set
	 */
	public void setGroupID(int groupID) {
		GroupID = groupID;
	}
	/**
	 * @return the groupInnerID
	 */
	public int getGroupInnerID() {
		return GroupInnerID;
	}
	/**
	 * @param groupInnerID the groupInnerID to set
	 */
	public void setGroupInnerID(int groupInnerID) {
		GroupInnerID = groupInnerID;
	}
	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	
	@Override
	public String toString() {
		return "VMDetail [ID=" + ID + ", VMNAME=" + VMNAME + ", OS=" + OS
				+ ", DISK=" + DISK + ", RAM=" + RAM + ", USERID=" + USERID
				+ ", DiskType=" + DiskType + ", IsParaVirt=" + IsParaVirt
				+ ", IsAutoScaling=" + IsAutoScaling + ", VirtType=" + VirtType
				+ ", Status=" + Status + ", Host=" + Host + ", GroupID="
				+ GroupID + ", GroupInnerID=" + GroupInnerID + "]";
	}
	/**
	 * @return the mapip
	 */
	public MACIP getMapip() {
		return mapip;
	}
	/**
	 * @param mapip the mapip to set
	 */
	public void setMapip(MACIP mapip) {
		this.mapip = mapip;
	}
	

}
