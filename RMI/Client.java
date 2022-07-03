
import javax.imageio.ImageIO;
import java.awt.*;
import java.awt.image.*;
import java.io.*;
import java.lang.reflect.Array;
import java.nio.file.Files;
import java.rmi.*;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Scanner;
import java.text.SimpleDateFormat;

public class Client {
    private static int[] auxint;
    private static int lenght;
    private static int width;
    private static int height;
    private static int length;

    public static void main(String[] args) {
        int serverQuantity;
        String imagePath;
        String imagePath2 = "";
        String imageExtension = "png";
        
        Hashtable<String, int[]> byteList;
        Hashtable<String, String> serverList;

        System.setSecurityManager(null);


        Scanner sc = new Scanner(System.in);
        System.out.println("Number of partitions : ");
        serverQuantity = sc.nextInt(); //quantidade de servidores
        sc.nextLine();
        if(serverQuantity > 5 || serverQuantity < 1){
            System.err.println("Must have between 1 and 5 servers");
            System.exit(-1);
        }
        
        serverList = new Hashtable<String, String>();
        for(int i = 1; i <= serverQuantity; i++){
            System.out.println("Inform the name of server " + i + " : ");
            String serverName = sc.nextLine();
            serverList.put(Integer.toString(i), serverName);
        }



        System.out.println("Image path : ");
        imagePath = sc.nextLine(); //caminho para a imagem
        
        if(imagePath.endsWith(".png")) {
        	imagePath2 = imagePath.substring(0, imagePath.length()-4);
        } else if(imagePath.endsWith(".jpeg")) {
        	imagePath2 = imagePath.substring(0, imagePath.length()-5);
        } else if(imagePath.endsWith(".jpg")) {
        	imagePath2 = imagePath.substring(0, imagePath.length()-4);
        } else {
        	System.out.println("File extension not allowed. Try again.");
        	System.exit(0);
        }
        
        System.out.println("File Extension: "+ imageExtension);
        System.out.println("File Path: "+ imagePath2);

        //Valor de inicio do tempo
        long inicio = System.currentTimeMillis();
        
        
        byteList = readImage(imagePath, serverQuantity);

        
        //Vetor de servidores
        Server servers[] = new Server[5];
        
        try {
        	//Definição da conexão com os servers pelo seu nome de registro
        	for(int i = 1; i <= serverQuantity; i++) {
        		servers[i-1] = (Server)Naming.lookup(serverList.get(Integer.toString(i)));
        		System.out.println("Lookup of server "+ serverList.get(Integer.toString(i)) + " done.");
        	}
        } catch (Exception e) {
            System.out.println("Caught an exception doing name lookup on server: "+ e);
            System.exit(-1);
        }

        
        try {
        	
        	//Threads para paralelismo de consulta ao servidor
        	Thread[] t = new Thread[serverQuantity];
        	
        	for(int i = 1; i <= serverQuantity; i++) {
        		t[i-1] = sendToServer(servers[i-1], byteList, i);
        		t[i-1].start();
        	}
        	
        	//Espera o fim de todas as threads
        	for(int i = 1; i <= serverQuantity; i++) {
        		t[i-1].join();
        	}
        	
        	//Montagem da imagem
        	buildImage(imagePath2, imageExtension, serverQuantity, byteList);
        	
        	//Valor do final do tempo
         	long fim = System.currentTimeMillis();

    	    SimpleDateFormat sdf = new SimpleDateFormat("mm:ss SSSSS");
    	    System.out.println("time spent running the conversion (mm:ss:ms): " + sdf.format(new java.util.Date(fim - inicio)));
        	  
        } catch (Exception e) {
        	System.out.println("Exception caught while getting grayscale conversion: "+ e);
        	e.printStackTrace();
        	System.exit(-1);
        }
        
        
    }


    public static Thread sendToServer(Server s, Hashtable<String, int[]> byteList, int index){
    	Thread t = new Thread() {
    	     
    	    @Override
    	    public void run() {
    	    	
    	    	try {
  
					byteList.put(Integer.toString(index), s.grayscaleConvertion(byteList.get(Integer.toString(index))));
					
					System.out.println("Call in server " + index + " complete.");
					
    	    	} catch (RemoteException e) {
			       	System.out.println("Exception caught while getting grayscale conversion in Thread: "+ e);
			       	e.printStackTrace();
		        	System.exit(-1);
				}
    	       
    	    }
    	  };
    	  
    	  return t;
    }
    
    public static Hashtable<String, int[]> readImage(String imagePath, int serverQuantity){
        File file = new File(imagePath);
        BufferedImage image = null;
        try {
            image = ImageIO.read(file);
        } catch (IOException e) {
            e.printStackTrace();
        }



        Hashtable<String ,int[]> byteList = new Hashtable<String ,int[]>();


        height = image.getHeight();
        width = image.getWidth();


        int[] imagePixels = new int[height*width];
        image.getRGB(0, 0, width, height, imagePixels, 0, width);

        auxint = imagePixels;
        length = imagePixels.length;
        System.out.println("Length : "+length);
        int part = length/serverQuantity;
        int [] aux = null;
        switch(serverQuantity){
            case 1:
                aux = imagePixels;
                byteList.put("1", aux);
                break;
            case 2:
                aux = Arrays.copyOfRange(imagePixels, 0, part);
                byteList.put("1", aux);
                aux = Arrays.copyOfRange(imagePixels, part, length);
                byteList.put("2", aux);
                break;
            case 3:
                aux = Arrays.copyOfRange(imagePixels, 0, part);
                byteList.put("1", aux);
                aux = Arrays.copyOfRange(imagePixels, part, 2*part);
                byteList.put("2", aux);
                aux = Arrays.copyOfRange(imagePixels, 2*part, length);
                byteList.put("3", aux);
                break;
            case 4:
                aux = Arrays.copyOfRange(imagePixels, 0, part);
                byteList.put("1", aux);
                aux = Arrays.copyOfRange(imagePixels, part, 2*part);
                byteList.put("2", aux);
                aux = Arrays.copyOfRange(imagePixels, 2*part, 3*part);
                byteList.put("3", aux);
                aux = Arrays.copyOfRange(imagePixels, 3*part, length);
                byteList.put("4", aux);
                break;
            case 5:
                aux = Arrays.copyOfRange(imagePixels, 0, part);
                byteList.put("1", aux);
                aux = Arrays.copyOfRange(imagePixels, part, 2*part);
                byteList.put("2", aux);
                aux = Arrays.copyOfRange(imagePixels, 2*part, 3*part);
                byteList.put("3", aux);
                aux = Arrays.copyOfRange(imagePixels, 3*part, 4*part);
                byteList.put("4", aux);
                aux = Arrays.copyOfRange(imagePixels, 4*part, length);
                byteList.put("5", aux);
                break;
            default:
                break;
        }
        return byteList;
    }

    public static void buildImage( String imagePath, String extension, int serverQuantity, Hashtable<String, int[]> byteList){

        int[] imageGray = null;

        System.out.println("Length = " + byteList.get("1").length);

        for(int i = 1; i <= serverQuantity; i++){

            int[] aux = null;
            if(imageGray == null){
                aux = new int[byteList.get(Integer.toString(i)).length];
                System.arraycopy(byteList.get(Integer.toString(i)), 0, aux, 0, byteList.get(Integer.toString(i)).length);

            }
            else{
                aux = new int[imageGray.length + byteList.get(Integer.toString(i)).length];
                System.arraycopy(imageGray, 0, aux, 0, imageGray.length);
                System.arraycopy(byteList.get(Integer.toString(i)), 0, aux, imageGray.length, byteList.get(Integer.toString(i)).length);
            }
            imageGray = aux;
            System.out.println("image gray size : " + imageGray.length + "   piece size : " + byteList.get(Integer.toString(i)).length);

            aux = null;
        }



        FileOutputStream fos;
        try {



            BufferedImage endImage = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
            int k = 0;
            for(int i = 0; i < height; i++){
                for(int j = 0; j < width; j++){
                    endImage.setRGB(j, i, imageGray[k]);
                    System.out.println("["+imageGray[k]+"]["+auxint[k]+"]");
                    k++;
                }
            }


            System.out.println(height + " - " + width + "\n" + endImage.getHeight() + " - " + endImage.getWidth());
            File file = new File(imagePath);
            
            ImageIO.write(endImage, extension, file);


        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

    }
}




