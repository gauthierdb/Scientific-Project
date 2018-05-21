package ascii_2nd_version;

import java.io.BufferedInputStream;
import java.io.BufferedWriter;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardWatchEventKinds;
import java.nio.file.WatchEvent;
import java.nio.file.WatchKey;
import java.nio.file.WatchService;
import java.util.logging.Level;
import java.util.logging.Logger;



/**
 *
 * @author AsifWasefi
 */
public class Ascii_2nd_version {
    
    static String inputFile = "input.txt";
    static String outputFile = "output.txt";
    static int doneBytes=0;

    public static void main(String[] args) {
        
        //make a thread which converts the bytes to Ascii
        InputToOutput thread1 = new InputToOutput("nodeMCU bytes naar Ascii");
        thread1.start();
        //make a thread which prints the converted bytes to screen
        OutputToPWM thread2 = new OutputToPWM("Ascii to screen");
        thread2.start();
        
        
    }
    
    /**
     * 
     * @filename The name of the file to read
     * @return the reader
     */
    public static DataInputStream inFile(String filename)
    {
        DataInputStream dis = null;
        try {
            
            dis = new DataInputStream(new BufferedInputStream(new FileInputStream(new File(filename))));
        } catch (FileNotFoundException ex) {
            Logger.getLogger(Ascii_2nd_version.class.getName()).log(Level.SEVERE, null, ex);
        }
        if (dis !=null) return dis;
        else return null;
        
    }
    
    /**
     * 
     * @filename the name of the file to write to
     * @return the writer
     */
    public static PrintWriter outFile (String filename)
    {
        PrintWriter pw = null;
        try {  
            pw = new PrintWriter(new BufferedWriter(new FileWriter(new File(filename),true)));
        } catch (IOException ex) {
            Logger.getLogger(Ascii_2nd_version.class.getName()).log(Level.SEVERE, null, ex);
        }
        if (pw != null) return pw;
        else return null;
    }
    
    /**
     * watches a folder for changes of modification of file
     * @folder folder The folder to watch
     * @suffix the file to watch
     */
    public static void watchFolder(String folder, String suffix) {
        
        try {
            Path path = Paths.get(folder);
            WatchService watchService = path.getFileSystem().newWatchService();
            path.register(watchService,StandardWatchEventKinds.ENTRY_MODIFY/*,StandardWatchEventKinds.ENTRY_CREATE,StandardWatchEventKinds.ENTRY_DELETE*/);
            
            convert();
            WatchKey key;
            while ((key = watchService.take()) != null) {
                for (WatchEvent<?> event : key.pollEvents()) {
                    Path changed = (Path) event.context();
                    if (changed.endsWith(suffix)) {
                        convert();
                    }
                }
                key.reset();
                
            }
        } catch (IOException | InterruptedException ex) {
            Logger.getLogger(Ascii_2nd_version.class.getName()).log(Level.SEVERE, null, ex);
        }


    }
    
    /**
     * It converts every byte in the reader to its Ascii equivalent
     * and writes it onto the output file
     */
    public static void convert()
    {
        //load the input file
        DataInputStream dis = inFile(inputFile);
        //load the output file
        PrintWriter pw = outFile(outputFile);
        try {
            if (dis !=null && pw !=null)
            {
                double remainingBytes=dis.available()-doneBytes;
                
                //skip the bytes which are already converted
                int dump = dis.skipBytes(doneBytes);
                
                //take the first byte and multiple by 9 and add the second byte
                for ( int i=0 ; i< (Math.floor(remainingBytes/2))  ; i++){
                    doneBytes=doneBytes+2;
                    byte firstByte  = dis.readByte();
                    byte secondByte  = dis.readByte();
                    //the if can be used to filter out special bytes like
                    //a space which is 32 in Ascii
                    if (true){
                        int firstByteAscii = firstByte;
                        int secondByteAscii= secondByte;
                        firstByteAscii=firstByteAscii*9+secondByteAscii;
                        pw.write(String.valueOf(firstByteAscii));pw.write("\n");
                    } 
                }
                    
                
                
            }
            dis.close();
            pw.close();
        } catch (IOException ex) {
            Logger.getLogger(Ascii_2nd_version.class.getName()).log(Level.SEVERE, null, ex);
        }   
        }
        
    }
    

