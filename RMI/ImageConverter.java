import java.rmi.*;

public interface ImageConverter {
	
	public byte[] grayscaleConvertion(byte[] imageBytes) throws RemoteException;
	
	
}
