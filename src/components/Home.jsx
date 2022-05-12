import React from "react"
import Layout from "@theme/Layout"
import Link from "@docusaurus/Link"
import bannerImg from "../../static/img/main.png"

import "../css/components/home.css"

function Banner() {
  return (
    <div className="container banner">
      <div className="banner_left">
        <img className="banner_logo" src="/SketchyBar/img/logo.svg" alt="SketchyBar logo" />
        <Link
          className="button button--secondary button--lg install_btn"
          to="/features"
        >
          Features
        </Link>
        <Link
          className="button button--secondary button--lg install_btn"
          to="/setup"
        >
          Install
        </Link>
      </div>
      <div className="banner__right">
        <img
          src={bannerImg}
          className="banner__right--screenshot"
          alt="SketchyBar banner screenshot"
        />
      </div>
    </div>
  )
}

export default function Home() {
  return (
    <Layout description="A highly customizable macOS statusbar replacement">
      <main className="homepage">
        <Banner />
      </main>
    </Layout>
  )
}
