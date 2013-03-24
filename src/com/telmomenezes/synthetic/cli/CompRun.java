package com.telmomenezes.synthetic.cli;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.cli.CommandLine;

import com.telmomenezes.synthetic.Generator;
import com.telmomenezes.synthetic.MetricsBag;
import com.telmomenezes.synthetic.Net;


public class CompRun extends Command {
	
	private List<String> textFiles(String directory) {
		List<String> textFiles = new ArrayList<String>();
		File dir = new File(directory);
		for (File file : dir.listFiles()) {
			if (file.getName().endsWith((".txt"))) {
				textFiles.add(file.getName());
		    }
		}
		
		return textFiles;
	}
	
    public boolean run(CommandLine cline) {
        if(!cline.hasOption("inet")) {
            setErrorMessage("input network file must be specified");
            return false;
        }
        if(!cline.hasOption("dir")) {
            setErrorMessage("directory must be specified");
            return false;
        }
        if(!cline.hasOption("out")) {
            setErrorMessage("out file must be specified");
            return false;
        }
        
        
        String netfile = cline.getOptionValue("inet");
        Net net = Net.load(netfile);

        String dir = cline.getOptionValue("dir");
        
        String outFile = cline.getOptionValue("out");
     	
     	int trials = 50;
     	if(cline.hasOption("trials")) {
            trials = new Integer(cline.getOptionValue("trials"));
        }
        
     	int bins = 100;
     	if(cline.hasOption("bins")) {
            bins = new Integer(cline.getOptionValue("bins"));
        }
     	
        boolean directed = true;
        
        System.out.println(net);
        
        List<String> prgFiles = textFiles(dir);
        int progCount = prgFiles.size();
        
        double[][] dists = new double[progCount][progCount];
        
        
        Map<String, Generator> genMap = new HashMap<String, Generator>();
        Map<String, MetricsBag> bagMap = new HashMap<String, MetricsBag>();
        for (String progFile : prgFiles) {
        	System.out.println("running " + progFile);
        	Generator gen = new Generator(net.getNodeCount(), net.getEdgeCount(), directed, trials);
    		gen.load(dir + "/" + progFile);
    		gen.run();
    		
    		genMap.put(progFile, gen);
    		
    		MetricsBag genBag = new MetricsBag(gen.getNet(), bins);
    		//MetricsBag genBag = new MetricsBag(net, bins);
    		bagMap.put(progFile, genBag);
        }
        
        int x = 0;
        int y = 0;
        for (String progFile1 : prgFiles) {
        	for (String progFile2 : prgFiles) {
        		System.out.println(progFile1 + " -> " + progFile2);
        		
        		Generator gen = genMap.get(progFile1);
        		MetricsBag bag = bagMap.get(progFile2);
        		
        		double dist = gen.computeFitness(bag, bins, false);
        		dists[x][y] = dist;
        		
        		System.out.println("distance: " + dist);
        		
        		x++;
        	}
        	y++;
        	x = 0;
        }
        
        System.out.println("Writing output file...");
        try {
        	FileWriter fstream = new FileWriter(outFile);
        	BufferedWriter out = new BufferedWriter(fstream);
        
        	for (y = 0; y < progCount; y++) {
        		for (x = 0; x < progCount; x++) {
        			if (x > 0) out.write(",");
        			out.write("" + dists[x][y]);
        		}
        		out.write("\n");
        	}
        
        	out.close();
        }
        catch (IOException e) {
        	e.printStackTrace();
        }
        
        System.out.println("done.");
        
        return true;
    }
}