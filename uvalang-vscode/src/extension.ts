// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
import * as vscode from 'vscode';
import * as cp from "child_process";
import * as fs from 'fs';
import * as os from 'os';
import * as path from 'path';

import { off } from 'process';

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

var analyserResult: AnalyserResult = new AnalyserResult([]);

class MyDefinitionProvider implements vscode.DefinitionProvider {
    constructor() {
    }

    provideDefinition(
        document: vscode.TextDocument,
        position: vscode.Position,
        token: vscode.CancellationToken
    ): vscode.ProviderResult<vscode.Definition> {
        console.log('DefinitionProvider called');

        const wordRange = document.getWordRangeAtPosition(position);
        if (!wordRange) {
            return null;
        }

        const word = document.getText(wordRange);

		for(const declaration of analyserResult.declarations) {
			if(declaration.name === word) {
				const startPos = new vscode.Position(declaration.location.line, declaration.location.column);
				const endPos = new vscode.Position(declaration.location.line, declaration.location.column + declaration.name.length);
				const range = new vscode.Range(startPos, endPos);
				const uri = vscode.Uri.file(declaration.location.file);

				return new vscode.Location(uri, range);
			}
		}

        return null; // Caso não encontre
    }
}

// This method is called when your extension is activated
// Your extension is activated the very first time the command is executed
export function activate(context: vscode.ExtensionContext) {
	context.subscriptions.push(
        vscode.languages.registerDefinitionProvider(
            { scheme: 'file', language: 'uva' },
            new MyDefinitionProvider()
        )
    );
	const classDecorationType = vscode.window.createTextEditorDecorationType({
		color: '#4EC9B0',
		fontWeight: 'bold',
		textDecoration: 'none',
	});

	function referenceRange(editor: vscode.TextEditor, reference: any, name: string, decorator: vscode.TextEditorDecorationType) : vscode.Range{
		const startPos = editor.document.positionAt(reference.offset);
		const endPos = editor.document.positionAt(reference.offset + name.length);
		const range = new vscode.Range(startPos, endPos);

		return range;
	}

	const updateDecorations = () => {
		console.log('updateDecorations...');

		const editor = vscode.window.activeTextEditor;

		if (!editor) return;

		if(editor.document.languageId !== 'uva') {
			console.log("cancel, not uva language");
			return;
		}

		analyserResult = new AnalyserResult([]);

		const content = editor.document.getText();

		const tmpFileName = path.join(os.tmpdir(), editor.document.fileName.substring(editor.document.fileName.lastIndexOf('/') + 1));

		fs.writeFileSync(tmpFileName, content);

		const start = new Date().getTime();

		var command = 'uvalang-analyser ' + editor.document.fileName + ' ' + tmpFileName;

		cp.exec(command, (error, stdout, stderr) => {
			const editor = vscode.window.activeTextEditor;

			if (!editor) return;

			if (error) {
				console.error(`${command} error in ${new Date().getTime() - start}ms`);
				return;
			}

			const now = new Date().getTime();

			console.log(`${command} success in ${now - start}ms`);
			//console.log(`stdout: ${stdout}`);

			const result = JSON.parse(stdout);

			for(const declaration of result.declarations) {
				const location = new Location(declaration.location.file, declaration.location.line, declaration.location.column, declaration.location.offset);
				const references = declaration.references.map((reference: any) => new Location(reference.file, reference.line, reference.column, reference.offset));

				analyserResult.declarations.push(new Declaration(declaration.name, location, references));
			}

			var ranges = [];
	
			for(const declaration of analyserResult.declarations) {
				//console.log(`decoring declaration for ${declaration.name} at ${JSON.stringify(declaration.location)}...`);
				ranges.push(referenceRange(editor, declaration.location, declaration.name, classDecorationType));
	
				for(const reference of declaration.references) {
					//console.log(`decorating reference ${JSON.stringify(reference)}...`);
	
					ranges.push(referenceRange(editor, reference, declaration.name, classDecorationType));
				}
			}
	
			editor.setDecorations(classDecorationType, ranges);
		});
	};

	const onDidChangeActiveTextEditor = vscode.window.onDidChangeActiveTextEditor((editor) => {
		updateDecorations();
	});

	const onDidChangeTextDocument = vscode.workspace.onDidChangeTextDocument((event) => {
		const editor = vscode.window.activeTextEditor;

		if (editor && event.document === editor.document) {
			updateDecorations();
		}
	});

	if (vscode.window.activeTextEditor) {
		updateDecorations();
	}

	context.subscriptions.push(onDidChangeActiveTextEditor, onDidChangeTextDocument);
}

// This method is called when your extension is deactivated
export function deactivate() {}
