import { themes as prismThemes } from 'prism-react-renderer';
import type { Config } from '@docusaurus/types';
import type * as Preset from '@docusaurus/preset-classic';

const config: Config = {
  title: 'Wildebeest Game Engine™',
  tagline: 'A multithreaded, high performance, fully functional game engine written in pure C, similar in speed to a Wildebeest™.',
  favicon: 'img/favicon.png',
  url: 'https://iwas-coder.github.io',
  baseUrl: '/wge',
  organizationName: 'iWas-Coder',
  projectName: 'wge',
  trailingSlash: false,
  onBrokenLinks: 'throw',
  onBrokenMarkdownLinks: 'warn',

  i18n: {
    defaultLocale: 'en',
    locales: ['en']
  },

  presets: [
    [
      'classic',
      {
        docs: {
          sidebarPath: './sidebars.ts',
          editUrl: 'https://github.com/iWas-Coder/wge/docs/www/',
          showLastUpdateTime: true
        },
        blog: {
          showReadingTime: true,
          editUrl: 'https://github.com/iWas-Coder/wge/docs/www/'
        },
        theme: {
          customCss: './src/css/custom.css',
        }
      } satisfies Preset.Options
    ]
  ],

  themeConfig: {
    image: 'img/docusaurus-social-card.jpg',
    navbar: {
      logo: {
        alt: 'WGE Logo',
        src: 'img/logo.png'
      },
      items: [
        {
          href: 'https://github.com/iWas-Coder/wge',
          label: 'GitHub',
          position: 'left',
        },
        {
          type: 'docSidebar',
          sidebarId: 'docs',
          position: 'left',
          label: 'Docs',
        },
        {
          to: '/blog',
          position: 'left',
          label: 'Blog'
        }
      ],
    },
    agolia: {
      appId: '',
      apiKey: '',
      indexName: '',
      contextualSearch: true
    },
    footer: {
      style: 'dark',
      links: [
        {
          title: 'Docs',
          items: [
            {
              label: 'What is WGE?',
              to: '/docs/overview/1.1-what-is-wge'
            },
            {
              label: 'Getting started',
              to: '/docs/getting-started/2.1-getting-started'
            }
          ]
        },
        {
          title: 'Community',
          items: [
            {
              label: 'Contributing',
              href: 'https://github.com/iWas-Coder/wge/blob/master/CONTRIBUTING.org'
            }
          ]
        },
        {
          title: 'More',
          items: [
            {
              label: 'GitHub',
              href: 'https://github.com/iWas-Coder/wge',
            }
          ]
        }
      ],
      copyright: `Copyright © 2023-${new Date().getFullYear()} Wasym Atieh Alonso. All rights reserved.`,
    },
    prism: {
      theme: prismThemes.github,
      darkTheme: prismThemes.dracula,
    }
  } satisfies Preset.ThemeConfig
};

export default config;
