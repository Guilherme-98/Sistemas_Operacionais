import java.rmi.*;

public interface Server extends Remote {
	
	public int[] grayscaleConvertion(int[] image) throws RemoteException;
	
	
}
