<template>
  <v-app>

    <v-navigation-drawer 
      app 
      clipped 
      :mini-variant="navbarIsExpanded" 
      :expand-on-hover="navbarIsExpanded" >

      <v-list>
        <v-list-item :to="'/lexer'" append>
          <v-list-item-icon>
            <v-icon>mdi-text-recognition</v-icon>
          </v-list-item-icon>

          <v-list-item-content>
            <v-list-item-title>Lexer</v-list-item-title>
          </v-list-item-content>

        </v-list-item>

        <v-divider/>
      </v-list>

    </v-navigation-drawer>

    <v-app-bar app color="primary" dark clipped-left flat>

      <v-app-bar-nav-icon @click="navbarIsExpanded = !navbarIsExpanded">
        
      </v-app-bar-nav-icon>

      <v-app-bar-title>Compilers Sandbox</v-app-bar-title>
      
      <v-spacer></v-spacer>

      <v-btn icon x-large color="white" @click="openGithubPage">
        <v-icon>mdi-github</v-icon>
      </v-btn>

    </v-app-bar>

    <v-main>
      <router-view />
    </v-main>

  </v-app>
</template>

<script lang="ts">
import Vue from "vue";

export default Vue.extend({
  name: "App",

  methods: {
    openGithubPage() {
      window.open("https://github.com/erikborella/compilers_sandbox", "_blank")
        ?.focus();
    }
  },

  data: () => ({
    navbarIsExpanded: false,
  }),
});
</script>
