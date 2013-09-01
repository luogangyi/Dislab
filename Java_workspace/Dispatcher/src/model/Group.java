package model;
import java.sql.*;

/**
 * 
 * @author lgy
 *
 */
public class Group {
	private int id;
	private int UserID;
	private Timestamp date;

	public int getId() {
		return id;
	}
	public void setId(int id) {
		this.id = id;
	}
	public int getUserID() {
		return UserID;
	}
	public void setUserID(int userID) {
		UserID = userID;
	}

	public Timestamp getDate() {
		return date;
	}
	public void setDate(Timestamp date) {
		this.date = date;
	}
	
}
