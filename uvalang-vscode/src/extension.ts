// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
import * as vscode from 'vscode';
import * as cp from "child_process";
import * as fs from 'fs';
import * as os from 'os';
import * as path from 'path';

import { off } from 'process';
import { buffer } from 'stream/consumers';

class Location {
	file: string;
	line: number;
	column: number;
	offset: number;

	constructor(file: string, line: number, column: number, offset: number) {
		this.file = file;
		this.line = line;
		this.column = column;
		this.offset = offset;
	}
}

class Declaration {
	name: string;
	location: Location;
	references: Location[];

	constructor(name: string, location: any, references: any) {
		this.name = name;
		this.location = location;
		this.references = references;
	}
};

class AnalyserResult {
	declarations: Declaration[];

	constructor(declarations: Declaration[]) {
		this.declarations = [];
	}
};

class AnalyserServer {
	private executable?: cp.ChildProcess;
	public onError?: (error: Error) => void;

	constructor() {
	}

	launch() {
		const start = Date.now();

		this.executable = cp.spawn('uvalang-analyser', ["--server"]);

		if(!this.executable) {
			this.throwErrorAtServer('unable to start');
		}

		const end = Date.now();

		console.log('uvalang-analyser server started in ' + (end - start) + 'ms');

		this.executable.on('close', (code) => {
			this.throwErrorAtServer(`exited with code ${code}`);
		});
	}

	analyse(document: vscode.TextDocument) : Promise<AnalyserResult> {
		if(document.languageId !== 'uva') {
			console.log("anlyse cancel, not uva language");

			return new Promise<AnalyserResult>((resolve, reject) => {
				resolve(new AnalyserResult([]));
			});
		}

		const now = Date.now();

		const content = document.getText();

		const tmpFileName = path.join(os.tmpdir(), document.fileName.substring(document.fileName.lastIndexOf('/') + 1));

		fs.writeFileSync(tmpFileName, content);

		var command = [document.fileName, tmpFileName];

		if(!this.writeCommand(command)) {
			return new Promise<AnalyserResult>((resolve, reject) => {
				this.throwErrorAtServer('unable to write command');
				reject(new Error('unable to write command'));
			});
		}

		return new Promise<AnalyserResult>((resolve, reject) => {
			const onData = (data: Buffer) => {
				const end = Date.now();
				const elapsed = end - now;

				//console.log(`stdout: ${data}`);

				const result = JSON.parse(data.toString());
			
				console.log(`${command} success in ${elapsed}ms (reported ${result.elapsed})`);

				const analyserResult = new AnalyserResult([]);
				
				for(const declaration of result.declarations) {
					const location = new Location(declaration.location.file, declaration.location.line, declaration.location.column, declaration.location.offset);
					const references = declaration.references.map((reference: any) => new Location(reference.file, reference.line, reference.column, reference.offset));
	
					analyserResult.declarations.push(new Declaration(declaration.name, location, references));
				}

				this.executable?.stdout?.off('data', onData);

				resolve(analyserResult);
			}

			const onError = (error: Error) => {
				const end = Date.now();
				console.log(`analyse canceled ${end - now}ms`);

				this.executable?.stdout?.off('error', onError);

				reject(error);
			}

			this.executable?.stdout?.on('data', onData);
			this.executable?.stdout?.on('error', onError);
		});
	}

	private throwErrorAtServer(message: string) {
		if(this.onError) {
			this.onError(new Error(message));
		}
	}

	private writeCommand(command: Array<string>) : Boolean {
		if(!this.executable?.stdin?.writable) {
			this.throwErrorAtServer('stdin is not writable');
			return false;
		}

		this.executable?.stdin?.write(command.join('\n') + '\n');
		return true;
	}
};

class MyDefinitionProvider implements vscode.DefinitionProvider {
	private analyserServer: AnalyserServer;

    constructor(analyserServer: AnalyserServer) {
		this.analyserServer = analyserServer;
    }

    provideDefinition(
        document: vscode.TextDocument,
        position: vscode.Position,
        token: vscode.CancellationToken
    ): vscode.ProviderResult<vscode.Definition> {
        //console.log('DefinitionProvider called');

        const wordRange = document.getWordRangeAtPosition(position);
		
        if (!wordRange) {
            return null;
        }

        const word = document.getText(wordRange);

		this.analyserServer.analyse(document).then((analyserResult) => {

			for(const declaration of analyserResult.declarations) {
				if(declaration.name === word) {
					const startPos = new vscode.Position(declaration.location.line, declaration.location.column);
					const endPos = new vscode.Position(declaration.location.line, declaration.location.column + declaration.name.length);
					const range = new vscode.Range(startPos, endPos);
					const uri = vscode.Uri.file(declaration.location.file);

					return new vscode.Location(uri, range);
				}
			}

		});

        return null;
    }
}

function referenceRange(editor: vscode.TextEditor, reference: any, name: string) : vscode.Range{
	const startPos = editor.document.positionAt(reference.offset);
	const endPos = editor.document.positionAt(reference.offset + name.length);
	const range = new vscode.Range(startPos, endPos);

	return range;
}

const classDecorationType = vscode.window.createTextEditorDecorationType({
	color: '#4EC9B0',
	fontWeight: 'bold',
	textDecoration: 'none',
});

function updateDecorations(analyserServer: AnalyserServer) {
	console.log('updateDecorations...');

	const editor = vscode.window.activeTextEditor;

	if(!editor) return;

	analyserServer.analyse(editor.document).then((analyserResult) => {
		var ranges = [];

		for(const declaration of analyserResult.declarations) {
			//console.log(`decoring declaration for ${declaration.name} at ${JSON.stringify(declaration.location)}...`);

			ranges.push(referenceRange(editor, declaration.location, declaration.name));

			for(const reference of declaration.references) {
				//console.log(`decorating reference ${JSON.stringify(reference)}...`);

				ranges.push(referenceRange(editor, reference, declaration.name));
			}
		}

		editor.setDecorations(classDecorationType, ranges);
	});
};

// This method is called when your extension is activated
// Your extension is activated the very first time the command is executed
export function activate(context: vscode.ExtensionContext) {

	var analyserServer = new AnalyserServer();
	analyserServer.launch();

	analyserServer.onError = (error) => {
		vscode.window.showErrorMessage(`${error.message}. The server will be restarted.`);
		analyserServer = new AnalyserServer();
		analyserServer.launch();
	};

	const updateCurrentDocumentDecorations = () => {
		if(analyserServer) {
			updateDecorations(analyserServer);
		}
	}

	const onDidChangeActiveTextEditor = vscode.window.onDidChangeActiveTextEditor((editor) => {
		updateCurrentDocumentDecorations();
	});

	const onDidChangeTextDocument = vscode.workspace.onDidChangeTextDocument((event) => {
		updateCurrentDocumentDecorations();
	});

	const registerDefinitionProvider = vscode.languages.registerDefinitionProvider({ scheme: 'file', language: 'uva' }, new MyDefinitionProvider(analyserServer));

	context.subscriptions.push(onDidChangeActiveTextEditor, onDidChangeTextDocument, registerDefinitionProvider);

	updateCurrentDocumentDecorations();
}

// This method is called when your extension is deactivated
export function deactivate() {}
