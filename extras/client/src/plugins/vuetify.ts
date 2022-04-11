import Vue from "vue";
import Vuetify from "vuetify/lib/framework";

import colors from 'vuetify/es5/util/colors'

Vue.use(Vuetify);

export default new Vuetify({
    theme: {
        themes: {
            light: {
                primary: colors.indigo.darken2,
                secondary: colors.indigo.lighten5,
                accent: colors.pink.base,
            }
        }
    }
});
