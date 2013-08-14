/**     ___           ___           ___                         ___           ___     
 *     /__/\         /  /\         /  /\         _____         /  /\         /__/|    
 *    |  |::\       /  /::\       /  /::|       /  /::\       /  /::\       |  |:|    
 *    |  |:|:\     /  /:/\:\     /  /:/:|      /  /:/\:\     /  /:/\:\      |  |:|    
 *  __|__|:|\:\   /  /:/~/::\   /  /:/|:|__   /  /:/~/::\   /  /:/  \:\   __|__|:|    
 * /__/::::| \:\ /__/:/ /:/\:\ /__/:/ |:| /\ /__/:/ /:/\:| /__/:/ \__\:\ /__/::::\____
 * \  \:\~~\__\/ \  \:\/:/__\/ \__\/  |:|/:/ \  \:\/:/~/:/ \  \:\ /  /:/    ~\~~\::::/
 *  \  \:\        \  \::/          |  |:/:/   \  \::/ /:/   \  \:\  /:/      |~~|:|~~ 
 *   \  \:\        \  \:\          |  |::/     \  \:\/:/     \  \:\/:/       |  |:|   
 *    \  \:\        \  \:\         |  |:/       \  \::/       \  \::/        |  |:|   
 *     \__\/         \__\/         |__|/         \__\/         \__\/         |__|/   
 *
 *  Description: 
 *				 
 *  DroppedFileAnalyzer.h, created by Marek Bereza on 13/08/2013.
 */

#include "Poco/RegularExpression.h"
using Poco::RegularExpression;


class DroppedFileAnalyzerListener {
public:
	
	
	virtual void droppedAddon(string addonName) {
		// this gets called back if addons are dropped (Can be called multiple times.)
		ofLogNotice() << "Unhandled droppedAddon() callback for " << addonName;
	}
	
	virtual void droppedSourceFolder(string src) {
		// called if you have dropped a single source folder containing at least one source file.
		ofLogNotice() << "Unhandled droppedSourceFolder() callback for " << src;
	}
	
	
	virtual void droppedSourceFiles(vector<string> &files) {
		// called if you drop a bunch of files (at least one must be a source file.
		ofLogNotice() << "Unhandled droppedSourceFiles() callback for " << files.size() << " files";
	}
	
	virtual void droppedProjectFolder(string path) {
		// called if you dropped a folder which contains at least one project file.
		ofLogNotice() << "Unhandled droppedProjectFolder() callback for " << path;
	}
	
	virtual void droppedProjectFile(string path) {
		// called if you dropped a single file and it's a project file
		ofLogNotice() << "Unhandled droppedProjectFile() callback for " << path;
	}
	
	virtual void droppedNothingOfImportance() {
		ofLogNotice() << "Dropped useless files ";
	}

};


class DroppedFileAnalyzer {
public:
		

	
	
	vector<string> sourceFileExts;
	vector<string> projectFileExts;
	
	DroppedFileAnalyzer() {
		sourceFileExts.push_back("h");
		sourceFileExts.push_back("hpp");
		sourceFileExts.push_back("cpp");
		sourceFileExts.push_back("cxx");
		sourceFileExts.push_back("hxx");
		sourceFileExts.push_back("c");
		sourceFileExts.push_back("m");
		sourceFileExts.push_back("mm");
		
		
		projectFileExts.push_back("xcodeproj");
		projectFileExts.push_back("cbp");
		projectFileExts.push_back("workspace");
		projectFileExts.push_back("vcxproj");
		projectFileExts.push_back("sln");
		
	}
	
	
	bool isSourceFile(string path) {
		string ext = ofToLower(ofFile(path).getExtension());
		for(int i = 0 ; i < sourceFileExts.size(); i++) {
			if(ext==sourceFileExts[i]) {
				return true;
			}
		}
		return false;
	}
	
	
	bool isProjectFile(string path) {
		string ext = ofToLower(ofFile(path).getExtension());
		for(int i = 0; i < projectFileExts.size(); i++) {
			if(ext==projectFileExts[i]) {
				return true;
			}
		}
		return false;
	}
	
	
	
	
	// non-recursive
	bool containsSourceFiles(vector<string> &files) {
				
		for(int i = 0; i < files.size(); i++) {
			
			if(isSourceFile(files[i])) return true;
		}
		// doesn't contain source files
		return false;
	}
	
	
	
	bool folderContainsSourceCode(string path) {
		ofDirectory dir;
		int numFiles = dir.listDir(path);
		for(int i = 0; i < numFiles; i++) {
			if(isSourceFile(dir[i].path())) return true;
		}
		return false;
	}
	
	void analyzeDroppedFiles(vector<string> &files, DroppedFileAnalyzerListener *listener) {

		
		// could be an addon/list of addons
		bool addedAddons = false;
		
		for(int i = 0; i < files.size(); i++) {
			string path = files[i];
			
			ofFile f(path);
			ofFile parent(f.getEnclosingDirectory());
			if(parent.getFileName()=="addons") {
				string addon = f.getFileName();
				listener->droppedAddon(addon);
				addedAddons = true;
			}
		}
		
		if(addedAddons) return;
		
		

		// exactly one folder containing at least one source file
		if(files.size()==1 && ofFile(files[0]).isDirectory() && folderContainsSourceCode(files[0])) {
			listener->droppedSourceFolder(files[0]);
			return;
		}
		
		
		
		// one or more source files and 0 or more other kinds of files

		for(int i = 0; i < files.size(); i++) {
			if(isSourceFile(files[i])) {
				listener->droppedSourceFiles(files);
				return;
			}
		}
		
		
		
		if(files.size()==1) {
			// could be a project folder with one or more project files (i.e. xcode and cb in one folder)
			if(ofFile(files[0]).isDirectory()) {
				
				ofDirectory dir;
				int numFiles = dir.listDir(files[0]);
				for(int i = 0; i < numFiles; i++) {
					if(isProjectFile(dir[i].path())) {
						listener->droppedProjectFolder(files[0]);
						return;
					}
				}
			}
		
			
			// project file
			if(isProjectFile(files[0])) {
				
				listener->droppedProjectFile(files[0]);
				vector<string> addons;
				getAddonsForProjectFile(files[0], addons);
				return;
			}
		}
		listener->droppedNothingOfImportance();
	}
	

	
	void getAddonsForProjectFile(string projectFile, vector<string> &outAddons) {


		// make sure we grab the vcxproj, not the solution
		if(projectFile.find(".sln")!=-1) {
			ofStringReplace(projectFile, ".sln", ".vcxproj");
		}
		// likewise, cbp, not workspace.
		if(projectFile.find(".workspace")!=-1) {
			ofStringReplace(projectFile, ".workspace", ".cbp");
		}

		
		// if xcode, look inside the package contents
		if(projectFile.find("xcodeproj")!=-1) {
			projectFile += "/project.pbxproj";
		}
		
		
		// read the xml file 
		ofBuffer buff = ofBufferFromFile(projectFile);
		string data = buff.getText();
		
		
		// this is the path separator - in windows code blocks, path separator can be / or \
		// on mac, sometimes the path ends in ';'
		string sep = "[\\/\\\\;]";

		
		
		// regex for paths to addons - addons/ofxXXX/
		string expression = "addons"+sep+"ofx[A-Za-z0-9\\-\\+]+"+sep;
		RegularExpression regEx(expression);
		RegularExpression::Match match;

		set<string> uniqueAddons;

		while(regEx.match(data, match) != 0) {
			
			string foundStr = data.substr(match.offset, match.length);
			data = data.substr(match.offset+match.length);
			
			
			// remove the preceding "addons/" and trailing "/" (or "\")
			string addon = foundStr.substr(string("addons/").size());
			addon =  addon.substr(0, addon.size()-1);
			
			//printf("%s\n", foundStr.c_str());
			uniqueAddons.insert(addon);
		}
		
		
//		printf("Found %d addons (regex: '%s')\n", uniqueAddons.size(), expression.c_str());
		for(set<string>::iterator it = uniqueAddons.begin(); it != uniqueAddons.end(); it++) {
			outAddons.push_back(*it);
			//printf("%s\n", (*it).c_str());
		}
	}
};






