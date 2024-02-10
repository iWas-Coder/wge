import clsx from 'clsx';
import Heading from '@theme/Heading';
import styles from './styles.module.css';

const imgRootPath = '/wge/img';

const featureList = [
  {
    title: 'Free/Libre',
    image: `${imgRootPath}/undraw_docusaurus_mountain.svg`,
    description: (
      <>
        As in <b>free speech</b> and as in <b>free bear</b>: You have the
        <a href='https://www.gnu.org/philosophy/free-sw.html'> freedom </a>
        to run, copy, distribute, study, change and improve the software.
      </>
    )
  },
  {
    title: 'Simple yet powerful',
    image: `${imgRootPath}/undraw_docusaurus_tree.svg`,
    description: (
      <>
        Made in <b>pure C</b>, not only the code is easy to understand,
        but it offers the flexibility for extensive customization
        according to your needs.
      </>
    )
  },
  {
    title: 'Fast. Blazingly fast',
    image: `${imgRootPath}/undraw_docusaurus_react.svg`,
    description: (
      <>
        DESCRIPTION_3
      </>
    )
  },
  {
    title: 'Cross-platform',
    image: `${imgRootPath}/undraw_docusaurus_react.svg`,
    description: (
      <>
        DESCRIPTION_4
      </>
    )
  },
  {
    title: 'Scripting made easy',
    image: `${imgRootPath}/undraw_docusaurus_mountain.svg`,
    description: (
      <>
        DESCRIPTION_5
      </>
    )
  },
  {
    title: 'Total control',
    image: `${imgRootPath}/undraw_docusaurus_tree.svg`,
    description: (
      <>
        DESCRIPTION_6
      </>
    )
  },
];

function Feature({title, image, description}) {
  return (
    <div className={clsx('col col--4')}>
      <div className='text--center'>
        <img className={ styles.featureImg } src={ image } alt={ title } />
      </div>
      <div className='text--center padding-horiz--md'>
        <Heading as='h3'>
          { title }
        </Heading>
        <p>{ description }</p>
      </div>
    </div>
  );
}

export default function HomepageFeatures() {
  return (
    <section className={ styles.features }>
      <div className='container'>
        <div className='row'>
          {featureList.map((props, idx) => (
            <Feature key={ idx } { ... props } />
          ))}
        </div>
      </div>
    </section>
  );
}
