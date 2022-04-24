<template>
  <v-container class="fill-height">
    <v-row justify="center" no-gutters>
      <v-btn color="primary" :loading="isLoadingTokens" @click="getTokens">
        Convert to Tokens
        <v-icon>mdi-send</v-icon>
      </v-btn>
    </v-row>

    <v-row class="fill-height">
      <v-col class="fill-height">
        <div id="code-editor" class="fill-height"></div>
      </v-col>

      <v-col>
        <v-row>
          <v-col v-for="(token, index) in tokens" :key="index" cols="6">
            <v-card elevation="1" @click="selectToken(token)">
              <v-card-title> {{ token.type }} </v-card-title>
              <v-card-text>
                <p>Attr: {{ token.attr }}</p>
                <p>
                  Location: {{token.location.start.line}}:{{token.location.start.column}} ; {{token.location.end.line}}:{{token.location.end.column}}
                </p>
              </v-card-text>
            </v-card>
          </v-col>
        </v-row>
      </v-col>
    </v-row>
  </v-container>
</template>

<script lang="ts">
import Vue from "vue";
import * as monaco from "monaco-editor";
import { LexerHttp, IToken } from "../shared/lexerHttp";

export default Vue.extend({
  name: "Lexer",

  data: () => ({
    editor: null as any,
    isLoadingTokens: false as boolean,
    tokens: [] as IToken[],
  }),

  mounted: function () {
    const exampleCode = "int main() {\n  return 0;\n}";

    const editorElement = document.getElementById("code-editor") as HTMLElement;
    this.editor = monaco.editor.create(editorElement, {
      theme: "vs-dark",
      language: "c",
      value: exampleCode,
    });
  },

  methods: {
    async getTokens() {
      this.isLoadingTokens = true;
      const lexerHttp = new LexerHttp();

      const tokens = await lexerHttp.getTokens(this.editor.getValue());
      this.tokens = tokens;

      this.isLoadingTokens = false;
    },

    selectToken(token: IToken) {
      this.editor.setSelection({
        startLineNumber: token.location.start.line,
        endLineNumber: token.location.end.line,
        startColumn: token.location.start.column,
        endColumn: token.location.end.column,
      });
    },
  },
});
</script>


<style>
</style>