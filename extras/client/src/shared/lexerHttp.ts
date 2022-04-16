import axios from "axios";

export interface IToken {
    type: string;
    position: {
        line: number;
        column: number;
    },
    attr: number;
}

export class LexerHttp {
    serverUrl = "http://localhost:8000";

    public async getTokens(code: string): Promise<IToken[]> {
        const tokens = await axios.post<IToken[]>(`${this.serverUrl}/lexer`, code);
        return tokens.data;
    }
}